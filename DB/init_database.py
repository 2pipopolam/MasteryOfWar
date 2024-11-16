import sqlite3
import os

def init_database():
    # Path to the database in DB folder
    db_path = "DB/game.db"
    
    # Create DB folder if it doesn't exist
    os.makedirs("DB", exist_ok=True)
    
    # Connect to database (creates new if doesn't exist)
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    
    # Create tables
    cursor.executescript("""
    -- Users table
    CREATE TABLE IF NOT EXISTS Users (
        user_id INTEGER PRIMARY KEY AUTOINCREMENT,
        nickname TEXT UNIQUE NOT NULL,
        password TEXT NOT NULL,
        salt TEXT NOT NULL,
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
        kills INTEGER DEFAULT 0,
        headshots INTEGER DEFAULT 0,
        deaths INTEGER DEFAULT 0,
        FOREIGN KEY (match_id) REFERENCES Matches(match_id),
        FOREIGN KEY (player_id) REFERENCES Users(user_id)
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

    -- Trigger to update statistics in scoreboard after match completion
    CREATE TRIGGER IF NOT EXISTS update_scoreboard_after_match
    AFTER UPDATE ON Matches
    WHEN NEW.status = 'finished' AND OLD.status = 'in_progress'
    BEGIN
        -- Update stats for player 1
        UPDATE Scoreboard
        SET total_kills = total_kills + (SELECT kills FROM MatchStats WHERE match_id = NEW.match_id AND player_id = NEW.player1_id),
            total_headshots = total_headshots + (SELECT headshots FROM MatchStats WHERE match_id = NEW.match_id AND player_id = NEW.player1_id),
            total_deaths = total_deaths + (SELECT deaths FROM MatchStats WHERE match_id = NEW.match_id AND player_id = NEW.player1_id),
            matches_played = matches_played + 1
        WHERE player_id = NEW.player1_id AND match_type_id = NEW.match_type_id;

        -- Update stats for player 2
        UPDATE Scoreboard
        SET total_kills = total_kills + (SELECT kills FROM MatchStats WHERE match_id = NEW.match_id AND player_id = NEW.player2_id),
            total_headshots = total_headshots + (SELECT headshots FROM MatchStats WHERE match_id = NEW.match_id AND player_id = NEW.player2_id),
            total_deaths = total_deaths + (SELECT deaths FROM MatchStats WHERE match_id = NEW.match_id AND player_id = NEW.player2_id),
            matches_played = matches_played + 1
        WHERE player_id = NEW.player2_id AND match_type_id = NEW.match_type_id;
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
