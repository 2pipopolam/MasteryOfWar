#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetworkStructs.h"

// Forward declarations
class APlayerSpawnPoint;
class UMasteryOfWarGameInstance;

#include "NetworkPlayerManager.generated.h"

UCLASS()
class ANetworkPlayerManager : public AActor
{
	GENERATED_BODY()

public:
	ANetworkPlayerManager();

	void Initialize(class UMasteryOfWarGameInstance* GameInst);
	void HandlePlayerJoined(int32 PlayerId, int32 SessionId);
	void HandlePlayerLeft(int32 PlayerId);
	void UpdatePlayerState(const FNetworkPlayerState& State);
	class AMasteryOfWarCharacter* SpawnNetworkPlayer(int32 PlayerId);
	AMasteryOfWarCharacter* GetPlayerCharacter(int32 PlayerId) const;

	
	void CleanupOccupiedSpawnPoints();

	void SynchronizePlayerStates();
	void ValidateSpawnPoints();
	FVector GetSpawnLocation(int32 PlayerId);

	void HandleSessionState(const FNetworkSessionState& State);

	UFUNCTION(BlueprintCallable, Category = "Network")
	const TMap<int32, AMasteryOfWarCharacter*>& GetNetworkPlayers() const { return NetworkPlayers; }
	
private:
	UPROPERTY()
	TMap<int32, AMasteryOfWarCharacter*> NetworkPlayers;

	UPROPERTY()
	UMasteryOfWarGameInstance* GameInstance;

	TArray<APlayerSpawnPoint*> OccupiedSpawnPoints;

	TArray<APlayerSpawnPoint*> GetAvailableSpawnPoints();
	void AssignSpawnPoints();
    
	UPROPERTY()
	TMap<int32, APlayerSpawnPoint*> PlayerSpawnPoints;
	

	AMasteryOfWarCharacter* SpawnPlayerAtPoint(int32 PlayerId, const APlayerSpawnPoint* SpawnPoint);
	APlayerSpawnPoint* FindValidSpawnPoint() const;
	void InitializeNetworkCharacter(AMasteryOfWarCharacter* Character, int32 PlayerId);
};