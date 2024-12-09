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

    if (NetworkPlayers.Contains(PlayerId))
    {
        UE_LOG(LogTemp, Warning, TEXT("Player %d already exists"), PlayerId);
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid World reference"));
        return;
    }

    // Получаем все точки спавна
    TArray<AActor*> AllSpawnPoints;
    UGameplayStatics::GetAllActorsOfClass(World, APlayerSpawnPoint::StaticClass(), AllSpawnPoints);
    
    UE_LOG(LogTemp, Warning, TEXT("Found %d spawn points"), AllSpawnPoints.Num());

    FVector SpawnLocation;
    FRotator SpawnRotation;

    // Если точек спавна нет, используем дефолтные позиции
    if (AllSpawnPoints.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No spawn points found, using default positions"));
        
        // Заданные позиции для разных игроков
        switch(PlayerId)
        {
            case 1:
                SpawnLocation = FVector(0.0f, -200.0f, 100.0f);
                break;
            case 2:
                SpawnLocation = FVector(0.0f, 200.0f, 100.0f);
                break;
            default:
                SpawnLocation = FVector(0.0f, 0.0f, 100.0f);
        }
        SpawnRotation = FRotator(0.0f, 0.0f, 0.0f);
    }
    else
    {
        // Используем существующие точки спавна
        int32 SpawnPointIndex = (PlayerId - 1) % AllSpawnPoints.Num();
        APlayerSpawnPoint* SelectedSpawnPoint = Cast<APlayerSpawnPoint>(AllSpawnPoints[SpawnPointIndex]);
        
        if (SelectedSpawnPoint)
        {
            SpawnLocation = SelectedSpawnPoint->GetActorLocation();
            SpawnRotation = SelectedSpawnPoint->GetActorRotation();
            SelectedSpawnPoint->SetOccupied(true);
            OccupiedSpawnPoints.Add(SelectedSpawnPoint);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to cast spawn point"));
            return;
        }
    }

    // Спавним персонажа
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    UE_LOG(LogTemp, Warning, TEXT("Attempting to spawn player %d at location: %s"), 
        PlayerId, *SpawnLocation.ToString());

    AMasteryOfWarCharacter* Character = World->SpawnActor<AMasteryOfWarCharacter>(
        AMasteryOfWarCharacter::StaticClass(),
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );

    if (Character)
    {
        NetworkPlayers.Add(PlayerId, Character);
        Character->SetPlayerId(PlayerId);

        if (GameInstance)
        {
            Character->InitializeForGameMode(GameInstance->GetCurrentGameModeConfig());
        }

        UE_LOG(LogTemp, Warning, TEXT("Successfully spawned player %d"), PlayerId);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn character for player %d"), PlayerId);
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
    
    UE_LOG(LogTemp, Warning, TEXT("Searching for spawn points. Found: %d"), SpawnPoints.Num());
    
    for (AActor* Actor : SpawnPoints)
    {
        if (APlayerSpawnPoint* SpawnPoint = Cast<APlayerSpawnPoint>(Actor))
        {
            UE_LOG(LogTemp, Warning, TEXT("Found spawn point at %s"), *SpawnPoint->GetActorLocation().ToString());
            
            if (!SpawnPoint->IsOccupied())
            {
                UE_LOG(LogTemp, Warning, TEXT("Selected unoccupied spawn point"));
                return SpawnPoint;
            }
        }
    }
    
    if (SpawnPoints.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("All spawn points occupied, returning first one"));
        return Cast<APlayerSpawnPoint>(SpawnPoints[0]);
    }
    
    UE_LOG(LogTemp, Error, TEXT("No spawn points found!"));
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
    UE_LOG(LogTemp, Warning, TEXT("Handling session state with %d players"), State.Players.Num());
    
    // Clean up occupied spawn points first
    CleanupOccupiedSpawnPoints();
    
    // Get available spawn points
    TArray<AActor*> SpawnPoints;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerSpawnPoint::StaticClass(), SpawnPoints);
    
    UE_LOG(LogTemp, Warning, TEXT("Found %d spawn points on map"), SpawnPoints.Num());
    
    if (UMasteryOfWarGameInstance* GameInst = Cast<UMasteryOfWarGameInstance>(GameInstance))
    {
        int32 LocalPlayerId = GameInst->GetNetworkClient()->GetPlayerId();
        
        // Process each player in session
        for (const auto& PlayerState : State.Players)
        {
            // Skip local player
            if (PlayerState.PlayerId == LocalPlayerId)
                continue;
                
            AMasteryOfWarCharacter* Character = nullptr;
            
            // Check if player already exists
            if (NetworkPlayers.Contains(PlayerState.PlayerId))
            {
                Character = NetworkPlayers[PlayerState.PlayerId];
                UE_LOG(LogTemp, Warning, TEXT("Updating existing player %d"), PlayerState.PlayerId);
            }
            else
            {
                APlayerSpawnPoint* SpawnPoint = nullptr;
                for (AActor* Actor : SpawnPoints)
                {
                    if (APlayerSpawnPoint* Point = Cast<APlayerSpawnPoint>(Actor))
                    {
                        if (!Point->IsOccupied())
                        {
                            SpawnPoint = Point;
                            break;
                        }
                    }
                }
                
                if (SpawnPoint)
                {
                    Character = SpawnPlayerAtPoint(PlayerState.PlayerId, SpawnPoint);
                    if (Character)
                    {
                        SpawnPoint->SetOccupied(true);
                        OccupiedSpawnPoints.Add(SpawnPoint);
                        NetworkPlayers.Add(PlayerState.PlayerId, Character);
                        UE_LOG(LogTemp, Warning, TEXT("Spawned new player %d at point %s"), 
                            PlayerState.PlayerId, *SpawnPoint->GetName());
                    }
                }
            }
            
            if (Character)
            {
                Character->UpdateFromNetworkState(PlayerState);
            }
        }
    }
}