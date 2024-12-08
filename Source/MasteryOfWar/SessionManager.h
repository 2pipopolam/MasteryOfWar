#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetworkStructs.h"
#include "SessionManager.generated.h"

UCLASS()
class MASTERYOFWAR_API ASessionManager : public AActor
{
	GENERATED_BODY()

public:
	ASessionManager();
    
	void Initialize(class UMasteryOfWarGameInstance* InGameInstance);
	bool JoinSession(int32 SessionId, const FString& Password);
	bool CreateSession(EGameMapType MapType, const FString& Password);
	void LeaveSession();
    
	// Обработчики сетевых событий
	void HandlePlayerJoined(int32 PlayerId);
	void HandlePlayerLeft(int32 PlayerId);
	void HandleSessionState(const FNetworkSessionState& State);

private:
	UPROPERTY()
	class UMasteryOfWarGameInstance* GameInstance;
    
	UPROPERTY()
	int32 CurrentSessionId;
    
	UPROPERTY()
	TArray<int32> ConnectedPlayers;
    
	void UpdateSessionState();
	void CleanupSession();
};
