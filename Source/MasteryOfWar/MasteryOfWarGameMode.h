#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameModeConfig.h"
#include "GameHUD.h"
#include "GameModesData.h"
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

	//Function for manual mode setting 
	UFUNCTION(BlueprintCallable, Category = "Game Mode")
	void SetGameMode(EGameModeType NewMode);

	// Function for obtaining the current mode 
	UFUNCTION(BlueprintPure, Category = "Game Mode")
	EGameModeType GetCurrentGameMode() const;

protected:
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

	// map mode identification
	EGameModeType DetermineGameModeFromMap(const FString& MapName);

	// loading the configuration for the specified mode
	bool LoadConfigForGameMode(EGameModeType ModeType);

	UPROPERTY(EditDefaultsOnly, Category = "Game Mode")
	FGameModeConfig CurrentModeConfig;

	// Data Asset with GM configs
	UPROPERTY(EditDefaultsOnly, Category = "Game Mode")
	TSoftObjectPtr<UGameModesData> GameModesData;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<class AGameHUD> GameHUDClass;

private:
	// current GM
	UPROPERTY()
	EGameModeType CurrentGameMode;
};