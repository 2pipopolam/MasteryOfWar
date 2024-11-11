// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameMode.h"
#include "GameHUD.h"
#include "MasteryOfWarGameMode.generated.h"

UCLASS(minimalapi)
class AMasteryOfWarGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMasteryOfWarGameMode();

protected:
	virtual void BeginPlay() override;
   
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<class AGameHUD> GameHUDClass;
};