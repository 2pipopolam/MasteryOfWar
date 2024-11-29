#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DamageConfig.h"
#include "DamageInterface.generated.h"

UINTERFACE(MinimalAPI)
class UDamageInterface : public UInterface
{
	GENERATED_BODY()
};

class MASTERYOFWAR_API IDamageInterface
{
	GENERATED_BODY()

public:
	virtual EHitZone GetHitZone(UPrimitiveComponent* HitComponent) const = 0;
};