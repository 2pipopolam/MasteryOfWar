#include "MasteryOfWarGameMode.h"
#include "MasteryOfWarCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "GameHUD.h"
#include "GameModesData.h"

AMasteryOfWarGameMode::AMasteryOfWarGameMode()
{
    // find HUD 
    static ConstructorHelpers::FClassFinder<AGameHUD> HUDClassFinder(TEXT("/Game/MofW/Blueprints/BP_GameHUD"));
    if (HUDClassFinder.Succeeded())
    {
        HUDClass = HUDClassFinder.Class;
        GameHUDClass = HUDClassFinder.Class;
    }
    else
    {
        HUDClass = AGameHUD::StaticClass();
        GameHUDClass = AGameHUD::StaticClass();
    }

    // upload Data Asset with GM configs
    static ConstructorHelpers::FObjectFinder<UGameModesData> ModesDataFinder(TEXT("/Game/MofW/DA_GameModes"));
    if (ModesDataFinder.Succeeded())
    {
        GameModesData = ModesDataFinder.Object;
        UE_LOG(LogTemp, Warning, TEXT("Game Modes Data Asset loaded successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load Game Modes Data Asset!"));
    }
    
    if (GameModesData.IsValid())
    {
        FGameModeConfig DefaultConfig = GameModesData->GetModeConfig(EGameModeType::RifleMode);
        if (DefaultConfig.CharacterClass)
        {
            DefaultPawnClass = DefaultConfig.CharacterClass;
            //UE_LOG(LogTemp, Warning, TEXT("Default character class set to: %s"), 
                //*DefaultConfig.CharacterClass->GetName());
        }
    }
}


void AMasteryOfWarGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    
    // Determine the mode by map and set the configuration 
    EGameModeType ModeType = DetermineGameModeFromMap(MapName);
    if (GameModesData.IsValid())
    {
        FGameModeConfig NewConfig = GameModesData->GetModeConfig(ModeType);
        SetGameModeConfig(NewConfig);
        
        // Set the character class from the configuration 
        if (NewConfig.CharacterClass)
        {
            DefaultPawnClass = NewConfig.CharacterClass;
            //UE_LOG(LogTemp, Warning, TEXT("Set character class from config: %s"), 
             //   *NewConfig.CharacterClass->GetName());
        }
    }
}


void AMasteryOfWarGameMode::BeginPlay()
{
    Super::BeginPlay();
}



void AMasteryOfWarGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    // Добавляем проверки и логирование
    if (!NewPlayer)
    {
        UE_LOG(LogTemp, Error, TEXT("PostLogin: NewPlayer is null"));
        return;
    }

    APawn* Pawn = NewPlayer->GetPawn();
    if (!Pawn)
    {
        UE_LOG(LogTemp, Error, TEXT("PostLogin: Player pawn is null"));
        return;
    }

    AMasteryOfWarCharacter* Character = Cast<AMasteryOfWarCharacter>(Pawn);
    if (!Character)
    {
        UE_LOG(LogTemp, Error, TEXT("PostLogin: Failed to cast pawn to MasteryOfWarCharacter"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Initializing character with mode: %s, weapon: %s"),
        *UEnum::GetValueAsString(CurrentModeConfig.ModeType),
        *UEnum::GetValueAsString(CurrentModeConfig.WeaponType));

    Character->InitializeForGameMode(CurrentModeConfig);
}



EGameModeType AMasteryOfWarGameMode::DetermineGameModeFromMap(const FString& MapName)
{
    // Добавляем логирование для отладки
    UE_LOG(LogTemp, Warning, TEXT("Determining game mode for map: %s"), *MapName);

    if (MapName.Contains(TEXT("PistolMap"), ESearchCase::IgnoreCase))
    {
        UE_LOG(LogTemp, Warning, TEXT("Selected Pistol Mode"));
        return EGameModeType::PistolMode;
    }
    else if (MapName.Contains(TEXT("GrenadeMap"), ESearchCase::IgnoreCase))
    {
        UE_LOG(LogTemp, Warning, TEXT("Selected Grenade Mode"));
        return EGameModeType::GrenadeMode;
    }
    else if (MapName.Contains(TEXT("RifleMap"), ESearchCase::IgnoreCase))
    {
        UE_LOG(LogTemp, Warning, TEXT("Selected Rifle Mode"));
        return EGameModeType::RifleMode;
    }
    
    // Логируем, если используется режим по умолчанию
    UE_LOG(LogTemp, Warning, TEXT("No specific mode found in map name, defaulting to Rifle Mode"));
    return EGameModeType::RifleMode;
}



void AMasteryOfWarGameMode::SetGameModeConfig(const FGameModeConfig& NewConfig)
{
    CurrentModeConfig = NewConfig;
    CurrentGameMode = NewConfig.ModeType;  // Обновляем текущий режим

    // Добавляем логирование
    UE_LOG(LogTemp, Warning, TEXT("Setting game mode config: Mode=%s, Weapon=%s"), 
        *UEnum::GetValueAsString(NewConfig.ModeType),
        *UEnum::GetValueAsString(NewConfig.WeaponType));

    // Устанавливаем класс персонажа
    if (NewConfig.CharacterClass)
    {
        DefaultPawnClass = NewConfig.CharacterClass;
        UE_LOG(LogTemp, Warning, TEXT("Set character class: %s"), *NewConfig.CharacterClass->GetName());
    }
}





void AMasteryOfWarGameMode::SetGameMode(EGameModeType NewMode)
{
    if (LoadConfigForGameMode(NewMode))
    {
        CurrentGameMode = NewMode;
    }
}

EGameModeType AMasteryOfWarGameMode::GetCurrentGameMode() const
{
    return CurrentGameMode;
}

bool AMasteryOfWarGameMode::LoadConfigForGameMode(EGameModeType ModeType)
{
    if (GameModesData.IsValid())
    {
        FGameModeConfig Config = GameModesData->GetModeConfig(ModeType);
        SetGameModeConfig(Config);
        return true;
    }
    return false;
}