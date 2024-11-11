#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "CrosshairSettingsTypes.h"
#include "CrosshairSaveSettings.generated.h"

UCLASS()
class MASTERYOFWAR_API UCrosshairSaveSettings : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FCrosshairSettings SavedSettings;
};