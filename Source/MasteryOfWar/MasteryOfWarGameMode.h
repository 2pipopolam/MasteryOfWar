#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameModeConfig.h"
#include "GameHUD.h"
#include "GameModesData.h"
#include "NetworkStructs.h"
#include "NetworkPlayerManager.h"
#include "MasteryOfWarGameMode.generated.h"

UCLASS(minimalapi)
class AMasteryOfWarGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AMasteryOfWarGameMode();

    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
    
    UFUNCTION(BlueprintCallable, Category = "Game Mode")
    void SetGameModeConfig(const FGameModeConfig& NewConfig);

    UFUNCTION(BlueprintPure, Category = "Game Mode")
    const FGameModeConfig& GetCurrentConfig() const { return CurrentModeConfig; }

    UFUNCTION(BlueprintCallable, Category = "Game Mode")
    void SetGameMode(EGameModeType NewMode);

    UFUNCTION(BlueprintPure, Category = "Game Mode")
    EGameModeType GetCurrentGameMode() const;

    // Network functionality
    UFUNCTION(BlueprintCallable, Category = "Network")
    ANetworkPlayerManager* GetPlayerManager() const { return PlayerManager; }

    // Player Management
    UFUNCTION(BlueprintCallable, Category = "Network|Players")
    void NotifyPlayerReady(class AMasteryOfWarCharacter* Character);

    UFUNCTION(BlueprintPure, Category = "Network")
    bool IsNetworkGame() const;

    UFUNCTION(BlueprintCallable, Category = "Game Mode")
    void ForceRespawnPlayer(AController* Controller);

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;

    // Map mode identification
    EGameModeType DetermineGameModeFromMap(const FString& MapName);

    // Config management
    bool LoadConfigForGameMode(EGameModeType ModeType);

    // Network initialization
    virtual void InitializeNetworking();
    virtual void SetupNetworkCallbacks();

    // Network event handlers
    UFUNCTION()
    virtual void HandleNewPlayerJoined(int32 PlayerId);
    
    UFUNCTION()
    virtual void HandlePlayerLeft(int32 PlayerId);
    
    UFUNCTION()
    virtual void HandleSessionState(const FNetworkSessionState& State);
    
    UFUNCTION()
    virtual void UpdatePlayerState(const FNetworkPlayerState& State);

    UPROPERTY(EditDefaultsOnly, Category = "Game Mode")
    FGameModeConfig CurrentModeConfig;

    UPROPERTY(EditDefaultsOnly, Category = "Game Mode")
    TSoftObjectPtr<UGameModesData> GameModesData;

    UPROPERTY(EditDefaultsOnly, Category = "HUD")
    TSubclassOf<class AGameHUD> GameHUDClass;
    
    UPROPERTY()
    ANetworkPlayerManager* PlayerManager;

private:
    UPROPERTY()
    EGameModeType CurrentGameMode;
};