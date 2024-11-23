#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WeaponConfig.h"
#include "WeaponInterface.generated.h"

UINTERFACE(MinimalAPI)
class UWeaponInterface : public UInterface
{
	GENERATED_BODY()
};

class MASTERYOFWAR_API IWeaponInterface
{
	GENERATED_BODY()

public:
	virtual const FBaseWeaponConfig& GetWeaponConfig() const = 0;
	virtual EWeaponType GetWeaponType() const = 0;
	virtual const FWeaponDamageConfig& GetDamageConfig() const = 0;
};