#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WeaponSystem.h"
#include "Bullet.h"
#include "BulletFireBehavior.generated.h"

class AMasteryOfWarCharacter;

UCLASS()
class MASTERYOFWAR_API UBulletFireBehavior : public UObject, public IFireBehavior
{
	GENERATED_BODY()

public:
	virtual void Fire(AWeapon* Weapon) override;
};
