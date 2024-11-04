#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WeaponConfig.h"
#include "WeaponSystem.h"
#include "WeaponFactory.generated.h"

UCLASS(Blueprintable)
class MASTERYOFWAR_API UWeaponFactory : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Weapon Factory")
	static AWeapon* CreateWeapon(UWorld* World, EWeaponType WeaponType, const FTransform& SpawnTransform);

private:
	static TSubclassOf<AWeapon> GetWeaponClass(EWeaponType WeaponType);
};