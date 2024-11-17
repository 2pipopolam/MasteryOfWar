#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ArmsPositionConfigWidget.h"
#include "ArmsConfigGameMode.generated.h"

UCLASS()
class MASTERYOFWAR_API AArmsConfigGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UArmsPositionConfigWidget> ConfigWidgetClass;
};