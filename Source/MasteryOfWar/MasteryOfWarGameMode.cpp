#include "MasteryOfWarGameMode.h"
#include "MasteryOfWarCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "GameHUD.h"
#include "GameModesData.h"
#include "MofWGameInstance.h"
#include "NetworkClient.h"
#include "PlayerSpawnPoint.h"
#include "NetworkPlayerManager.h"


AMasteryOfWarGameMode::AMasteryOfWarGameMode()
{
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
        }
    }
}

void AMasteryOfWarGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    
    EGameModeType ModeType = DetermineGameModeFromMap(MapName);
    if (GameModesData.IsValid())
    {
        FGameModeConfig NewConfig = GameModesData->GetModeConfig(ModeType);
        SetGameModeConfig(NewConfig);
        
        if (NewConfig.CharacterClass)
        {
            DefaultPawnClass = NewConfig.CharacterClass;
        }
    }
}

void AMasteryOfWarGameMode::BeginPlay()
{
    Super::BeginPlay();
    InitializeNetworking();
}

void AMasteryOfWarGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (PlayerManager)
    {
        PlayerManager->Destroy();
        PlayerManager = nullptr;
    }
    Super::EndPlay(EndPlayReason);
}

void AMasteryOfWarGameMode::InitializeNetworking()
{
    // Spawn NetworkPlayerManager
    FActorSpawnParameters SpawnParams;
    PlayerManager = GetWorld()->SpawnActor<ANetworkPlayerManager>(SpawnParams);
    
    if (UMasteryOfWarGameInstance* GameInstance = Cast<UMasteryOfWarGameInstance>(GetGameInstance()))
    {
        PlayerManager->Initialize(GameInstance);
        
        if (NetworkClient* Client = GameInstance->GetNetworkClient())
        {
            SetupNetworkCallbacks();
        }
    }
}


void AMasteryOfWarGameMode::SetupNetworkCallbacks()
{
    if (UMasteryOfWarGameInstance* GameInstance = Cast<UMasteryOfWarGameInstance>(GetGameInstance()))
    {
        if (NetworkClient* Client = GameInstance->GetNetworkClient())
        {
            Client->OnPlayerJoined.AddUObject(this, &AMasteryOfWarGameMode::HandleNewPlayerJoined);
            Client->OnPlayerLeft.AddUObject(this, &AMasteryOfWarGameMode::HandlePlayerLeft);
            Client->OnPlayerStateReceived.AddUObject(this, &AMasteryOfWarGameMode::UpdatePlayerState);
            
            Client->OnSessionStateReceived.AddDynamic(this, &AMasteryOfWarGameMode::HandleSessionState);
        }
    }
}




void AMasteryOfWarGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (!NewPlayer)
    {
        UE_LOG(LogTemp, Error, TEXT("PostLogin: NewPlayer is null"));
        return;
    }

    if (UMasteryOfWarGameInstance* GameInstance = Cast<UMasteryOfWarGameInstance>(GetGameInstance()))
    {
        if (NetworkClient* Client = GameInstance->GetNetworkClient())
        {
            if (Client->GetCurrentSessionId() != -1)
            {
                Client->RequestSessionState();
                
                if (PlayerManager)
                {
                    int32 PlayerId = Client->GetPlayerId();
                    PlayerManager->HandlePlayerJoined(PlayerId, Client->GetCurrentSessionId());
                }
            }
        }
    }

    if (APawn* Pawn = NewPlayer->GetPawn())
    {
        if (AMasteryOfWarCharacter* Character = Cast<AMasteryOfWarCharacter>(Pawn))
        {
            Character->InitializeForGameMode(CurrentModeConfig);
        }
    }
}




void AMasteryOfWarGameMode::Logout(AController* Exiting)
{
    if (AMasteryOfWarCharacter* Character = Cast<AMasteryOfWarCharacter>(Exiting->GetPawn()))
    {
        if (PlayerManager)
        {
            PlayerManager->HandlePlayerLeft(Character->GetPlayerId());
        }
    }
    Super::Logout(Exiting);
}

void AMasteryOfWarGameMode::HandleNewPlayerJoined(int32 PlayerId)
{
    if (PlayerManager)
    {
        PlayerManager->HandlePlayerJoined(PlayerId, 0);
    }
}

void AMasteryOfWarGameMode::HandlePlayerLeft(int32 PlayerId)
{
    if (PlayerManager)
    {
        PlayerManager->HandlePlayerLeft(PlayerId);
    }
}

void AMasteryOfWarGameMode::HandleSessionState(const FNetworkSessionState& State)
{
    if (PlayerManager)
    {
        for (const auto& PlayerState : State.Players)
        {
            PlayerManager->UpdatePlayerState(PlayerState);
        }
    }
}

void AMasteryOfWarGameMode::UpdatePlayerState(const FNetworkPlayerState& State)
{
    if (PlayerManager)
    {
        PlayerManager->UpdatePlayerState(State);
    }
}



EGameModeType AMasteryOfWarGameMode::DetermineGameModeFromMap(const FString& MapName)
{
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
    
    UE_LOG(LogTemp, Warning, TEXT("No specific mode found in map name, defaulting to Rifle Mode"));
    return EGameModeType::RifleMode;
}




void AMasteryOfWarGameMode::SetGameModeConfig(const FGameModeConfig& NewConfig)
{
    CurrentModeConfig = NewConfig;
    CurrentGameMode = NewConfig.ModeType;

    UE_LOG(LogTemp, Warning, TEXT("Setting game mode config: Mode=%s, Weapon=%s"), 
        *UEnum::GetValueAsString(NewConfig.ModeType),
        *UEnum::GetValueAsString(NewConfig.WeaponType));

    if (NewConfig.CharacterClass && PlayerManager)
    {
        DefaultPawnClass = NewConfig.CharacterClass;
        UE_LOG(LogTemp, Warning, TEXT("Set character class: %s"), *NewConfig.CharacterClass->GetName());

        // Get players through public method
        const TMap<int32, AMasteryOfWarCharacter*>& Players = PlayerManager->GetNetworkPlayers();
        for (const auto& Pair : Players)
        {
            if (AMasteryOfWarCharacter* Character = Pair.Value)
            {
                Character->InitializeForGameMode(NewConfig);
            }
        }
    }
}




void AMasteryOfWarGameMode::SetGameMode(EGameModeType NewMode)
{
    if (LoadConfigForGameMode(NewMode))
    {
        CurrentGameMode = NewMode;
        
        // Уведомляем других игроков об изменении режима
        if (UMasteryOfWarGameInstance* GameInstance = Cast<UMasteryOfWarGameInstance>(GetGameInstance()))
        {
            if (NetworkClient* Client = GameInstance->GetNetworkClient())
            {
                Client->RequestSessionState(); // Запрашиваем обновление состояния сессии
            }
        }
    }
}

bool AMasteryOfWarGameMode::LoadConfigForGameMode(EGameModeType ModeType)
{
    if (GameModesData.IsValid())
    {
        FGameModeConfig Config = GameModesData->GetModeConfig(ModeType);
        SetGameModeConfig(Config);

        // Проверяем, нужно ли инициализировать сетевые компоненты
        if (!PlayerManager)
        {
            InitializeNetworking();
        }

        return true;
    }
    return false;
}

EGameModeType AMasteryOfWarGameMode::GetCurrentGameMode() const
{
    return CurrentGameMode;
}




void AMasteryOfWarGameMode::NotifyPlayerReady(AMasteryOfWarCharacter* Character)
{
    if (!Character) return;

    if (UMasteryOfWarGameInstance* GameInstance = Cast<UMasteryOfWarGameInstance>(GetGameInstance()))
    {
        if (NetworkClient* Client = GameInstance->GetNetworkClient())
        {
            FNetworkPlayerState State;
            State.PlayerId = Character->GetPlayerId();
            State.Position = Character->GetActorLocation();
            State.Rotation = Character->GetActorRotation();
            State.bIsCrouching = Character->bIsCrouched;
            State.bIsWalking = Character->IsWalking();

            if (AWeapon* Weapon = Character->GetCurrentWeapon())
            {
                State.bIsFiring = Weapon->IsFiring();
                State.bIsReloading = Weapon->IsReloading();
                State.CurrentAmmo = Weapon->GetCurrentAmmo();
                State.WeaponType = Weapon->GetWeaponType();
            }

            Client->SendPlayerState(State);
        }
    }
}



bool AMasteryOfWarGameMode::IsNetworkGame() const
{
    if (UMasteryOfWarGameInstance* GameInstance = Cast<UMasteryOfWarGameInstance>(GetGameInstance()))
    {
        return GameInstance->IsConnectedToServer();
    }
    return false;
}




void AMasteryOfWarGameMode::ForceRespawnPlayer(AController* Controller)
{
    if (!Controller || !PlayerManager) return;
    
    if (AMasteryOfWarCharacter* Character = Cast<AMasteryOfWarCharacter>(Controller->GetPawn()))
    {
        int32 PlayerId = Character->GetPlayerId();
        
        if (AMasteryOfWarCharacter* NewCharacter = PlayerManager->SpawnNetworkPlayer(PlayerId))
        {
            Character->Destroy();
            
            Controller->Possess(NewCharacter);
            
            NewCharacter->InitializeForGameMode(CurrentModeConfig);
            NewCharacter->SetPlayerId(PlayerId);
            NotifyPlayerReady(NewCharacter);
        }
    }
}