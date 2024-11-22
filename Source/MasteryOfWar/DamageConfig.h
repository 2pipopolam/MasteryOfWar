#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DamageConfig.generated.h"

UENUM(BlueprintType)
enum class EHitZone : uint8
{
	Head        UMETA(DisplayName = "Head"),
	Body        UMETA(DisplayName = "Body"),
	Arms        UMETA(DisplayName = "Arms"),
	Legs        UMETA(DisplayName = "Legs")
};

USTRUCT(BlueprintType)
struct FHitZoneMultipliers
{
	GENERATED_BODY()

	// damage multipliers
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float HeadMultiplier = 4.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float BodyMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float ArmsMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float LegsMultiplier = 0.75f;

	float GetMultiplier(EHitZone HitZone) const
	{
		switch (HitZone)
		{
		case EHitZone::Head: return HeadMultiplier;
		case EHitZone::Body: return BodyMultiplier;
		case EHitZone::Arms: return ArmsMultiplier;
		case EHitZone::Legs: return LegsMultiplier;
		default: return 1.0f;
		}
	}
};

USTRUCT(BlueprintType)
struct FModeDamageConfig
{
	GENERATED_BODY()

	// basic damage for certain game mode
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float BaseDamage = 25.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	FHitZoneMultipliers HitZoneMultipliers;
};