#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Blueprint/UserWidget.h"
#include "CrosshairSettingsMenu.h"
#include "CrosshairSettingsGameMode.generated.h"

UCLASS()
class MASTERYOFWAR_API ACrosshairSettingsGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> SettingsMenuClass;
};