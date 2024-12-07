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
    if (!NetworkPlayers.Contains(PlayerId))
    {
        CleanupOccupiedSpawnPoints();
        
        if (APlayerSpawnPoint* SpawnPoint = FindValidSpawnPoint())
        {
            if (AMasteryOfWarCharacter* Character = SpawnPlayerAtPoint(PlayerId, SpawnPoint))
            {
                NetworkPlayers.Add(PlayerId, Character);
                SpawnPoint->SetOccupied(true);
                OccupiedSpawnPoints.Add(SpawnPoint);
                UE_LOG(LogTemp, Warning, TEXT("Spawned network player %d at spawn point"), PlayerId);
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("No available spawn points for player %d"), PlayerId);
        }
    }
}



void ANetworkPlayerManager::CleanupOccupiedSpawnPoints()
{
    for (APlayerSpawnPoint* SpawnPoint : OccupiedSpawnPoints)
    {
        if (SpawnPoint && SpawnPoint->IsValidLowLevel())
        {
            SpawnPoint->SetOccupied(false);
        }
    }
    OccupiedSpawnPoints.Empty();
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