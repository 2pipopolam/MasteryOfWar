import sqlite3
import os

def init_database():
    # Path to the database in DB folder
    db_path = "DB/game.db"
    
    # Check if database already exists with proper structure
    if os.path.exists(db_path):
        try:
            conn = sqlite3.connect(db_path)
            cursor = conn.cursor()
            
            # Check if required tables exist
            cursor.execute("SELECT name FROM sqlite_master WHERE type='table' AND name='Users'")
            if cursor.fetchone():
                print("Database already initialized, skipping initialization")
                conn.close()
                return
            conn.close()
        except sqlite3.Error as e:
            print(f"Error checking database: {e}")      
    

    os.makedirs("DB", exist_ok=True)
    
   
    # Create tables
    cursor.executescript("""
    -- Users table
    CREATE TABLE IF NOT EXISTS Users (
        user_id INTEGER PRIMARY KEY AUTOINCREMENT,
        nickname TEXT UNIQUE NOT NULL,
        password TEXT NOT NULL,
        salt TEXT NOT NULL,
        avatar_path TEXT DEFAULT 'default_avatar',
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    );

    -- Match types table
    CREATE TABLE IF NOT EXISTS MatchTypes (
        type_id INTEGER PRIMARY KEY AUTOINCREMENT,
        type_name TEXT UNIQUE NOT NULL CHECK (type_name IN ('Pistol', 'Rifle', 'Sniper', 'Grenade'))
    );

    -- Matches table
    CREATE TABLE IF NOT EXISTS Matches (
        match_id INTEGER PRIMARY KEY AUTOINCREMENT,
        match_type_id INTEGER,
        player1_id INTEGER,
        player2_id INTEGER,
        status TEXT CHECK (status IN ('waiting', 'in_progress', 'finished')),
        started_at TIMESTAMP,
        finished_at TIMESTAMP,
        FOREIGN KEY (match_type_id) REFERENCES MatchTypes(type_id),
        FOREIGN KEY (player1_id) REFERENCES Users(user_id),
        FOREIGN KEY (player2_id) REFERENCES Users(user_id)
    );

    -- Match statistics table
    CREATE TABLE IF NOT EXISTS MatchStats (
        stats_id INTEGER PRIMARY KEY AUTOINCREMENT,
        match_id INTEGER,
        player_id INTEGER,
        match_type_id INTEGER,
        kills INTEGER DEFAULT 0,
        headshots INTEGER DEFAULT 0,
        deaths INTEGER DEFAULT 0,
        FOREIGN KEY (match_id) REFERENCES Matches(match_id),
        FOREIGN KEY (player_id) REFERENCES Users(user_id),
        FOREIGN KEY (match_type_id) REFERENCES MatchTypes(type_id)
    );

    -- Overall player statistics table (scoreboard)
    CREATE TABLE IF NOT EXISTS Scoreboard (
        scoreboard_id INTEGER PRIMARY KEY AUTOINCREMENT,
        player_id INTEGER,
        match_type_id INTEGER,
        total_kills INTEGER DEFAULT 0,
        total_headshots INTEGER DEFAULT 0,
        total_deaths INTEGER DEFAULT 0,
        matches_played INTEGER DEFAULT 0,
        FOREIGN KEY (player_id) REFERENCES Users(user_id),
        FOREIGN KEY (match_type_id) REFERENCES MatchTypes(type_id),
        UNIQUE(player_id, match_type_id)
    );
    """)

    # Create triggers
    cursor.executescript("""
    -- Trigger to create scoreboard entries when a new user is registered
    CREATE TRIGGER IF NOT EXISTS update_scoreboard_after_user_creation
    AFTER INSERT ON Users
    BEGIN
        INSERT INTO Scoreboard (player_id, match_type_id, total_kills, total_headshots, total_deaths, matches_played)
        SELECT NEW.user_id, type_id, 0, 0, 0, 0
        FROM MatchTypes;
    END;
    """)

    # Insert initial match types
    cursor.executescript("""
    INSERT OR IGNORE INTO MatchTypes (type_name) VALUES
        ('Pistol'),
        ('Rifle'),
        ('Sniper'),
        ('Grenade');
    """)

    # Save changes and close connection
    conn.commit()
    conn.close()

if __name__ == "__main__":
    init_database()
    print("Database initialized successfully!")
