#include "DatabaseManager.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/SecureHash.h"

FDatabaseManager* FDatabaseManager::Instance = nullptr;

FDatabaseManager& FDatabaseManager::Get()
{
    if (!Instance)
    {
        Instance = new FDatabaseManager();
    }
    return *Instance;
}

FDatabaseManager::FDatabaseManager() : DB(nullptr)
{
    Connect();
}

FDatabaseManager::~FDatabaseManager()
{
    Close();
}

bool FDatabaseManager::Connect()
{
    FString DBPath = FPaths::ProjectDir() / TEXT("DB/game.db");
    return sqlite3_open(TCHAR_TO_UTF8(*DBPath), &DB) == SQLITE_OK;
}

void FDatabaseManager::Close()
{
    if (DB)
    {
        sqlite3_close(DB);
        DB = nullptr;
    }
}

FString FDatabaseManager::HashPassword(const FString& Password, const FString& Salt)
{
    return FMD5::HashAnsiString(*(Password + Salt));
}

FString FDatabaseManager::GenerateSalt()
{
    return FGuid::NewGuid().ToString();
}

int32 FDatabaseManager::AuthenticateUser(const FString& Nickname, const FString& Password)
{
    if (!DB) return -1;

    FString Query = FString::Printf(TEXT("SELECT user_id, password, salt FROM Users WHERE nickname = '%s';"),
        *Nickname);
    
    sqlite3_stmt* Statement;
    if (sqlite3_prepare_v2(DB, TCHAR_TO_UTF8(*Query), -1, &Statement, nullptr) == SQLITE_OK)
    {
        if (sqlite3_step(Statement) == SQLITE_ROW)
        {
            int32 UserId = sqlite3_column_int(Statement, 0);
            FString StoredPassword = UTF8_TO_TCHAR(sqlite3_column_text(Statement, 1));
            FString Salt = UTF8_TO_TCHAR(sqlite3_column_text(Statement, 2));
            
            if (HashPassword(Password, Salt) == StoredPassword)
            {
                sqlite3_finalize(Statement);
                return UserId;
            }
        }
        sqlite3_finalize(Statement);
    }
    return -1;
}

int32 FDatabaseManager::RegisterUser(const FString& Nickname, const FString& Password)
{
    if (!DB) return -1;

    FString Salt = GenerateSalt();
    FString HashedPassword = HashPassword(Password, Salt);
    
    FString Query = FString::Printf(TEXT("INSERT INTO Users (nickname, password, salt) VALUES ('%s', '%s', '%s');"),
        *Nickname, *HashedPassword, *Salt);
    
    char* ErrorMessage = nullptr;
    if (sqlite3_exec(DB, TCHAR_TO_UTF8(*Query), nullptr, nullptr, &ErrorMessage) == SQLITE_OK)
    {
        return sqlite3_last_insert_rowid(DB);
    }
    
    sqlite3_free(ErrorMessage);
    return -1;
}

bool FDatabaseManager::UpdateUserAvatar(int32 UserId, const FString& NewAvatarPath)
{
   if (!DB) return false;

   FString Query = FString::Printf(TEXT("UPDATE Users SET avatar_path = '%s' WHERE user_id = %d;"),
       *NewAvatarPath, UserId);

   char* ErrorMessage = nullptr;
   int Result = sqlite3_exec(DB, TCHAR_TO_UTF8(*Query), nullptr, nullptr, &ErrorMessage);
   
   if (Result != SQLITE_OK)
   {
       sqlite3_free(ErrorMessage);
       return false;
   }
   return true;
}

bool FDatabaseManager::UpdateUserNickname(int32 UserId, const FString& NewNickname)
{
   if (!DB) return false;

   // check if nickname already exists 
   FString CheckQuery = FString::Printf(TEXT("SELECT COUNT(*) FROM Users WHERE nickname = '%s' AND user_id != %d;"),
       *NewNickname, UserId);
   
   sqlite3_stmt* Statement;
   if (sqlite3_prepare_v2(DB, TCHAR_TO_UTF8(*CheckQuery), -1, &Statement, nullptr) == SQLITE_OK)
   {
       if (sqlite3_step(Statement) == SQLITE_ROW)
       {
           int Count = sqlite3_column_int(Statement, 0);
           sqlite3_finalize(Statement);
           if (Count > 0)
           {
               return false; //nickname exists 
           }
       }
   }

   FString UpdateQuery = FString::Printf(TEXT("UPDATE Users SET nickname = '%s' WHERE user_id = %d;"),
       *NewNickname, UserId);

   char* ErrorMessage = nullptr;
   int Result = sqlite3_exec(DB, TCHAR_TO_UTF8(*UpdateQuery), nullptr, nullptr, &ErrorMessage);
   
   if (Result != SQLITE_OK)
   {
       sqlite3_free(ErrorMessage);
       return false;
   }
   return true;
}

FDatabaseManager::FUserData FDatabaseManager::GetUserData(int32 UserId)
{
   FUserData UserData;
   UserData.UserId = UserId;

   if (!DB) return UserData;

   FString Query = FString::Printf(TEXT("SELECT nickname, avatar_path FROM Users WHERE user_id = %d;"), UserId);
   
   sqlite3_stmt* Statement;
   if (sqlite3_prepare_v2(DB, TCHAR_TO_UTF8(*Query), -1, &Statement, nullptr) == SQLITE_OK)
   {
       if (sqlite3_step(Statement) == SQLITE_ROW)
       {
           const unsigned char* Nickname = sqlite3_column_text(Statement, 0);
           const unsigned char* AvatarPath = sqlite3_column_text(Statement, 1);
           
           UserData.Nickname = UTF8_TO_TCHAR(Nickname);
           UserData.AvatarPath = UTF8_TO_TCHAR(AvatarPath);
       }
       sqlite3_finalize(Statement);
   }

   return UserData;
}