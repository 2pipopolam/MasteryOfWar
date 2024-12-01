#include "AuthGameMode.h"
#include "Misc/Base64.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Containers/StringConv.h"
#include "MofWGameInstance.h"


#if PLATFORM_LINUX
    #include <sqlite3.h>
#else
    #include "sqlite3.h"
#endif

// Callback func for SQLite
static int SQLiteCallback(void* data, int argc, char** argv, char** azColName)
{
    TArray<FString>* Results = static_cast<TArray<FString>*>(data);
    for(int i = 0; i < argc; i++)
    {
        Results->Add(UTF8_TO_TCHAR(argv[i] ? argv[i] : "NULL"));
    }
    return 0;
}

AAuthGameMode::AAuthGameMode()
{
    Database = nullptr;
    FString DBPath = FPaths::ProjectDir() + TEXT("Server/DB/game.db");
    
    // open or create new DB
    auto PathConverter = StringCast<ANSICHAR>(*DBPath);
    if (sqlite3_open(PathConverter.Get(), &Database) != SQLITE_OK)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to open/create database"));
        Database = nullptr;
    }
    else 
    {
        // create Users table if doesnt exist
        const char* createTableQuery = 
            "CREATE TABLE IF NOT EXISTS Users ("
            "user_id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "nickname TEXT UNIQUE NOT NULL,"
            "password TEXT NOT NULL,"
            "salt TEXT NOT NULL,"
            "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";
            
        char* errMsg = nullptr;
        
        if (sqlite3_exec(Database, createTableQuery, nullptr, nullptr, &errMsg) != SQLITE_OK)
        {
            FString ErrorMessage = UTF8_TO_TCHAR(errMsg);
            UE_LOG(LogTemp, Error, TEXT("Failed to create table: %s"), *ErrorMessage);
            sqlite3_free(errMsg);
        }
    }
    
    bIsLoggedIn = false;
    CurrentUserId = -1;
}

AAuthGameMode::~AAuthGameMode()
{
    if (Database)
    {
        sqlite3_close(Database);
        Database = nullptr;
    }
}


void AAuthGameMode::BeginPlay()
{
    Super::BeginPlay();

    if (AuthWidgetClass)
    {
        APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
        if (PlayerController)
        {
            AuthWidget = CreateWidget<UAuthWidget>(PlayerController, AuthWidgetClass);
            if (AuthWidget)
            {
                AuthWidget->AddToViewport();

                PlayerController->SetInputMode(FInputModeUIOnly());
                PlayerController->bShowMouseCursor = true;
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to create Auth Widget"));
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Auth Widget Class is not set in AuthGameMode"));
    }
}



bool AAuthGameMode::ExecuteQuery(const FString& Query, TArray<FString>& Results)
{
    if (!Database)
        return false;
        
    char* errMsg = nullptr;
    auto QueryConverter = StringCast<ANSICHAR>(*Query);
    
    if (sqlite3_exec(Database, QueryConverter.Get(), SQLiteCallback, &Results, &errMsg) != SQLITE_OK)
    {
        FString ErrorMessage = UTF8_TO_TCHAR(errMsg);
        UE_LOG(LogTemp, Error, TEXT("SQL error: %s"), *ErrorMessage);
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool AAuthGameMode::ExecuteNonQuery(const FString& Query)
{
    if (!Database)
        return false;
        
    char* errMsg = nullptr;
    auto QueryConverter = StringCast<ANSICHAR>(*Query);
    
    if (sqlite3_exec(Database, QueryConverter.Get(), nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        FString ErrorMessage = UTF8_TO_TCHAR(errMsg);
        UE_LOG(LogTemp, Error, TEXT("SQL error: %s"), *ErrorMessage);
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

FString AAuthGameMode::GenerateSalt() const
{
    uint8 RandomBytes[16];
    for (int32 i = 0; i < 16; i++)
    {
        RandomBytes[i] = FMath::RandRange(0, 255);
    }
    return FBase64::Encode(RandomBytes, 16);
}

FString AAuthGameMode::HashPassword(const FString& Password, const FString& Salt) const
{
    FString SaltedPassword = Password + Salt;
    TArray<uint8> Bytes;
    Bytes.SetNum(SaltedPassword.Len() * sizeof(TCHAR));
    FMemory::Memcpy(Bytes.GetData(), *SaltedPassword, Bytes.Num());
    
    return FBase64::Encode(Bytes.GetData(), Bytes.Num());
}

bool AAuthGameMode::VerifyPassword(const FString& InputPassword, const FString& StoredHash, const FString& StoredSalt) const
{
    FString InputHash = HashPassword(InputPassword, StoredSalt);
    return InputHash.Equals(StoredHash);
}

bool AAuthGameMode::AuthSignUp(const FString& Nickname, const FString& Password)
{
    if (!Database || Nickname.IsEmpty() || Password.IsEmpty())
        return false;

    // check if user exists
    TArray<FString> CheckResults;
    FString CheckQuery = FString::Printf(TEXT("SELECT user_id FROM Users WHERE nickname = '%s'"), *Nickname);
    
    ExecuteQuery(CheckQuery, CheckResults);
    if (CheckResults.Num() > 0)
    {
        return false; // user exists
    }

    FString Salt = GenerateSalt();
    FString HashedPassword = HashPassword(Password, Salt);

    // create new user
    FString Query = FString::Printf(TEXT("INSERT INTO Users (nickname, password, salt) VALUES ('%s', '%s', '%s')"),
        *Nickname, *HashedPassword, *Salt);
    
    if (ExecuteNonQuery(Query))
    {
        return AuthLogin(Nickname, Password);
    }
    
    return false;
}


bool AAuthGameMode::AuthLogin(const FString& Nickname, const FString& Password)
{
    if (!Database || bIsLoggedIn)
        return false;

    TArray<FString> Results;
    FString Query = FString::Printf(TEXT("SELECT user_id, password, salt FROM Users WHERE nickname = '%s'"), *Nickname);
    
    if (ExecuteQuery(Query, Results) && Results.Num() >= 3)
    {
        FString StoredHash = Results[1];  // password
        FString StoredSalt = Results[2];  // salt

        if (VerifyPassword(Password, StoredHash, StoredSalt))
        {
            CurrentUser = Nickname;
            CurrentUserId = FCString::Atoi(*Results[0]);  // user_id
            bIsLoggedIn = true;

            // Update GameInstance with the current user ID
            if (UMasteryOfWarGameInstance* GameInstance = Cast<UMasteryOfWarGameInstance>(GetGameInstance()))
            {
                GameInstance->SetCurrentUserId(CurrentUserId);
                UE_LOG(LogTemp, Log, TEXT("User %s logged in, ID: %d, Stats initialized"), *Nickname, CurrentUserId);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to get GameInstance for user stats initialization"));
            }

            return true;
        }
    }
    
    return false;
}


void AAuthGameMode::AuthLogout()
{
    bIsLoggedIn = false;
    CurrentUser = "";
    CurrentUserId = -1;
}