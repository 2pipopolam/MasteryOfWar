#include "NetworkPlayerManager.h"
#include "MasteryOfWarCharacter.h"
#include "PlayerSpawnPoint.h"
#include "MofWGameInstance.h"
#include "GameModeConfig.h"
#include "Kismet/GameplayStatics.h"

ANetworkPlayerManager::ANetworkPlayerManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ANetworkPlayerManager::Initialize(UMasteryOfWarGameInstance* GameInst)
{
    GameInstance = GameInst;
}



void ANetworkPlayerManager::HandlePlayerJoined(int32 PlayerId, int32 SessionId)
{
    UE_LOG(LogTemp, Warning, TEXT("Handling player join: PlayerId=%d, SessionId=%d"), PlayerId, SessionId);

    if (AMasteryOfWarCharacter* ExistingCharacter = NetworkPlayers.FindRef(PlayerId))
    {
        UE_LOG(LogTemp, Warning, TEXT("Player %d already exists, updating"), PlayerId);
        return;
    }

    CleanupOccupiedSpawnPoints();

    if (APlayerSpawnPoint* SpawnPoint = FindValidSpawnPoint())
    {
        if (AMasteryOfWarCharacter* Character = SpawnPlayerAtPoint(PlayerId, SpawnPoint))
        {
            NetworkPlayers.Add(PlayerId, Character);
            SpawnPoint->SetOccupied(true);
            OccupiedSpawnPoints.Add(SpawnPoint);

            UE_LOG(LogTemp, Warning, TEXT("Spawned player %d at point %s"), 
                PlayerId, *SpawnPoint->GetName());

            if (GameInstance && GameInstance->GetNetworkClient())
            {
                GameInstance->GetNetworkClient()->RequestSessionState();
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("No available spawn points for player %d!"), PlayerId);
    }
}



void ANetworkPlayerManager::CleanupOccupiedSpawnPoints()
{
    for (int32 i = OccupiedSpawnPoints.Num() - 1; i >= 0; --i)
    {
        if (APlayerSpawnPoint* SpawnPoint = OccupiedSpawnPoints[i])
        {
            bool bStillInUse = false;
            for (const auto& Pair : NetworkPlayers)
            {
                if (Pair.Value && Pair.Value->GetActorLocation().Equals(SpawnPoint->GetActorLocation(), 1.0f))
                {
                    bStillInUse = true;
                    break;
                }
            }
            
            if (!bStillInUse)
            {
                SpawnPoint->SetOccupied(false);
                OccupiedSpawnPoints.RemoveAt(i);
                UE_LOG(LogTemp, Warning, TEXT("Freed spawn point %s"), *SpawnPoint->GetName());
            }
        }
    }
}





void ANetworkPlayerManager::HandlePlayerLeft(int32 PlayerId)
{
    if (AMasteryOfWarCharacter* Character = NetworkPlayers.FindRef(PlayerId))
    {
        Character->Destroy();
        NetworkPlayers.Remove(PlayerId);
        UE_LOG(LogTemp, Warning, TEXT("Removed network player %d"), PlayerId);
    }
}

AMasteryOfWarCharacter* ANetworkPlayerManager::SpawnNetworkPlayer(int32 PlayerId)
{
    if (APlayerSpawnPoint* SpawnPoint = FindValidSpawnPoint())
    {
        return SpawnPlayerAtPoint(PlayerId, SpawnPoint);
    }
    return nullptr;
}



AMasteryOfWarCharacter* ANetworkPlayerManager::SpawnPlayerAtPoint(int32 PlayerId, const APlayerSpawnPoint* SpawnPoint)
{
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    if (AMasteryOfWarCharacter* Character = GetWorld()->SpawnActor<AMasteryOfWarCharacter>(
        AMasteryOfWarCharacter::StaticClass(),
        SpawnPoint->GetActorLocation(),
        SpawnPoint->GetActorRotation(),
        SpawnParams))
    {
        InitializeNetworkCharacter(Character, PlayerId);
        return Character;
    }
    return nullptr;
}

void ANetworkPlayerManager::InitializeNetworkCharacter(AMasteryOfWarCharacter* Character, int32 PlayerId)
{
    // Set up character for network play
    Character->SetPlayerId(PlayerId);
    
    // Initialize weapon and other components based on game mode
    if (GameInstance)
    {
        Character->InitializeForGameMode(GameInstance->GetCurrentGameModeConfig());
    }
}

void ANetworkPlayerManager::UpdatePlayerState(const FNetworkPlayerState& State)
{
    if (AMasteryOfWarCharacter* Character = NetworkPlayers.FindRef(State.PlayerId))
    {
        Character->UpdateFromNetworkState(State);
    }
}

AMasteryOfWarCharacter* ANetworkPlayerManager::GetPlayerCharacter(int32 PlayerId) const
{
    return NetworkPlayers.FindRef(PlayerId);
}



APlayerSpawnPoint* ANetworkPlayerManager::FindValidSpawnPoint() const
{
    TArray<AActor*> SpawnPoints;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerSpawnPoint::StaticClass(), SpawnPoints);
    
    // Сначала ищем свободные точки спавна
    for (AActor* Actor : SpawnPoints)
    {
        if (APlayerSpawnPoint* SpawnPoint = Cast<APlayerSpawnPoint>(Actor))
        {
            if (!SpawnPoint->IsOccupied())
            {
                return SpawnPoint;
            }
        }
    }
    
    // Если свободных нет, берем случайную точку
    if (SpawnPoints.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, SpawnPoints.Num() - 1);
        return Cast<APlayerSpawnPoint>(SpawnPoints[RandomIndex]);
    }
    
    return nullptr;
}


void ANetworkPlayerManager::ValidateSpawnPoints()
{
    TArray<AActor*> SpawnPoints;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerSpawnPoint::StaticClass(), SpawnPoints);
    
    if (SpawnPoints.Num() < 2)
    {
        UE_LOG(LogTemp, Error, TEXT("Not enough spawn points on map! Minimum required: 2"));
    }
}

TArray<APlayerSpawnPoint*> ANetworkPlayerManager::GetAvailableSpawnPoints()
{
    TArray<APlayerSpawnPoint*> Available;
    TArray<AActor*> SpawnPoints;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerSpawnPoint::StaticClass(), SpawnPoints);
    
    for (AActor* Actor : SpawnPoints)
    {
        if (APlayerSpawnPoint* SpawnPoint = Cast<APlayerSpawnPoint>(Actor))
        {
            if (!SpawnPoint->IsOccupied())
            {
                Available.Add(SpawnPoint);
            }
        }
    }
    return Available;
}

void ANetworkPlayerManager::AssignSpawnPoints()
{
    TArray<APlayerSpawnPoint*> Available = GetAvailableSpawnPoints();
    int32 Index = 0;
    
    for (auto& Pair : NetworkPlayers)
    {
        if (!PlayerSpawnPoints.Contains(Pair.Key) && Index < Available.Num())
        {
            PlayerSpawnPoints.Add(Pair.Key, Available[Index]);
            Available[Index]->SetOccupied(true);
            Index++;
        }
    }
}

void ANetworkPlayerManager::SynchronizePlayerStates()
{
    if (!GameInstance || !GameInstance->GetNetworkClient()) return;
    
    for (auto& Pair : NetworkPlayers)
    {
        if (AMasteryOfWarCharacter* Character = Pair.Value)
        {
            FNetworkPlayerState State;
            State.PlayerId = Pair.Key;
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
            
            GameInstance->GetNetworkClient()->SendPlayerState(State);
        }
    }
}




void ANetworkPlayerManager::HandleSessionState(const FNetworkSessionState& State)
{
    if (State.Players.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Received empty session state"));
        return;
    }

    for (const auto& PlayerState : State.Players)
    {
        if (PlayerState.WeaponType == EWeaponType::None)
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid weapon type for player %d"), PlayerState.PlayerId);
            continue;
        }

        HandlePlayerJoined(PlayerState.PlayerId, State.SessionId);
        UpdatePlayerState(PlayerState);
    }
}