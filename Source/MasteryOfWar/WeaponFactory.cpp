#include "WeaponFactory.h"
#include "AK47.h"

AWeapon* UWeaponFactory::CreateWeapon(UWorld* World, EWeaponType WeaponType, const FTransform& SpawnTransform)
{
	if (!World)
	{
		return nullptr;
	}

	TSubclassOf<AWeapon> WeaponClass = GetWeaponClass(WeaponType);
	if (!WeaponClass)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AWeapon* Weapon = World->SpawnActor<AWeapon>(WeaponClass, SpawnTransform, SpawnParams);
	return Weapon;
}

TSubclassOf<AWeapon> UWeaponFactory::GetWeaponClass(EWeaponType WeaponType)
{
	switch (WeaponType)
	{
	case EWeaponType::AK47:
		return AAK47::StaticClass();
		// Add cases for other weapons when implemented
	default:
		return nullptr;
	}
}