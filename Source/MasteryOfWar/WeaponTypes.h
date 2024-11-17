#pragma once

#include "CoreMinimal.h"
#include "WeaponTypes.generated.h"

UENUM(BlueprintType)
enum class EWeaponTypeEnum : uint8
{
	None UMETA(DisplayName = "None"),
	AK47 UMETA(DisplayName = "AK47"),
	DesertEagle UMETA(DisplayName = "Desert Eagle")
};