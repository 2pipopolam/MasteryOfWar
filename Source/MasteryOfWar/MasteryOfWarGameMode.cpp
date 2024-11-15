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
            UE_LOG(LogTemp, Warning, TEXT("Default character class set to: %s"), 
                *DefaultConfig.CharacterClass->GetName());
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
            UE_LOG(LogTemp, Warning, TEXT("Set character class from config: %s"), 
                *NewConfig.CharacterClass->GetName());
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

    // Initialize the character with the current configuration 
    if (AMasteryOfWarCharacter* Character = Cast<AMasteryOfWarCharacter>(NewPlayer->GetPawn()))
    {
        Character->InitializeForGameMode(CurrentModeConfig);
    }
}


EGameModeType AMasteryOfWarGameMode::DetermineGameModeFromMap(const FString& MapName)
{
    // Define mode by card name
    if (MapName.Contains(TEXT("PistolMap")))
    {
        UE_LOG(LogTemp, Warning, TEXT("DetermineGameModeFromMap: Found PistolMode map"));
        return EGameModeType::PistolMode;
    }
    else if (MapName.Contains(TEXT("ThirdPerson")))
    {
        UE_LOG(LogTemp, Warning, TEXT("DetermineGameModeFromMap: Found ThirdPerson map, using RifleMode"));
        return EGameModeType::RifleMode;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("DetermineGameModeFromMap: Default to RifleMode"));
    return EGameModeType::RifleMode;
}


void AMasteryOfWarGameMode::SetGameModeConfig(const FGameModeConfig& NewConfig)
{
    CurrentModeConfig = NewConfig;

    // set default char class
    if (NewConfig.CharacterClass)
    {
        DefaultPawnClass = NewConfig.CharacterClass;
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