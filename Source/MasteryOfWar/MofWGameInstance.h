#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "NetworkClient.h"
#include "GameModeConfig.h"
#include "NetworkPlayerManager.h"
#include "NetworkMapLoader.h"
#include "MofWGameInstance.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNetworkErrorSignature, int32, ErrorCode, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionCreatedSignature, int32, SessionId);

UCLASS()


class MASTERYOFWAR_API UMasteryOfWarGameInstance : public UGameInstance,  public INetworkMapLoader
{
    GENERATED_BODY()

public:
    UMasteryOfWarGameInstance(const FObjectInitializer& ObjectInitializer);


    virtual void LoadNetworkMap(const FString& MapPath, int32 SessionId) override;

    // Networking methods
    UFUNCTION(BlueprintCallable, Category = "Networking")
    bool InitializeNetworking(const FString& IPAddress, int32 Port);

    UFUNCTION(BlueprintCallable, Category = "Networking")
    bool CreateGameSession(EGameMapType MapType, const FString& Password = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Networking")
    bool JoinGameSession(int32 SessionId, const FString& Password = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Networking")
    void DisconnectFromServer();

    UFUNCTION(BlueprintPure, Category = "Networking")
    bool IsConnectedToServer() const;

    NetworkClient* GetNetworkClient() const { return NetworkConnection.Get(); }

    // Game Mode Management
    UFUNCTION(BlueprintCallable, Category = "Game Mode")
    void SetCurrentGameMode(EGameMapType GameMode) { CurrentGameMode = GameMode; }

    UFUNCTION(BlueprintPure, Category = "Game Mode")
    EGameMapType GetCurrentGameMode() const { return CurrentGameMode; }

    // User Management methods
    UFUNCTION(BlueprintCallable, Category = "User Management")
    void SetCurrentUserId(int32 UserId);

    UFUNCTION(BlueprintPure, Category = "User Management")
    int32 GetCurrentUserId() const { return CurrentUserId; }

    UFUNCTION(BlueprintPure, Category = "User Management")
    bool IsUserLoggedIn() const { return CurrentUserId > 0; }

    // Network delegates
    UPROPERTY(BlueprintAssignable, Category = "Networking")
    FOnNetworkErrorSignature OnNetworkError;

    UPROPERTY(BlueprintAssignable, Category = "Networking")
    FOnSessionCreatedSignature OnNetworkCreatedSession;


    const FGameModeConfig& GetCurrentGameModeConfig() const 
    { 
        return GameModeConfigs[static_cast<int32>(CurrentGameMode)]; 
    }
    

    
    void SetPlayerManager(ANetworkPlayerManager* Manager);
    AMasteryOfWarCharacter* SpawnNetworkPlayer(int32 PlayerId);

private:
    int32 CurrentUserId;
    EGameMapType CurrentGameMode;
    TUniquePtr<NetworkClient> NetworkConnection;

    UPROPERTY()
    TArray<FGameModeConfig> GameModeConfigs;
    
    // Network event handlers
    void HandleNetworkError(int32 ErrorCode, const FString& ErrorMessage);
    void HandleSessionCreated(int32 SessionId);
    void HandlePlayerState(const FNetworkPlayerState& State);
    void HandleShot(const FNetworkShotInfo& ShotInfo);
    void HandleGrenadeThrow(const FNetworkGrenadeInfo& GrenadeInfo);

    void HandleSessionState(const FNetworkSessionState& State);


    UPROPERTY()
    class ANetworkPlayerManager* PlayerManager;
};