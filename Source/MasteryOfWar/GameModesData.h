#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameModeConfig.h"
#include "GameModesData.generated.h"

UCLASS(BlueprintType, Blueprintable)
class MASTERYOFWAR_API UGameModesData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// all modes configs
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Modes")
	TMap<EGameModeType, FGameModeConfig> ModesConfig;

	// get config of certain mode
	UFUNCTION(BlueprintCallable, Category = "Game Modes")
	FGameModeConfig GetModeConfig(EGameModeType ModeType)
	{
		if (const FGameModeConfig* Config = ModesConfig.Find(ModeType))
		{
			return *Config;
		}
		return FGameModeConfig();
	}

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);
	}
#endif
};