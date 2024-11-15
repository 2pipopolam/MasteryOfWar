#include "WeaponFactory.h"
#include "AK47.h"
#include "DesertEagle.h"


AWeapon* UWeaponFactory::CreateWeapon(UWorld* World, EWeaponType WeaponType, const FTransform& SpawnTransform)
{
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateWeapon: World is null!"));
		return nullptr;
	}

	UE_LOG(LogTemp, Warning, TEXT("CreateWeapon: Creating weapon of type %d"), (int32)WeaponType);

	TSubclassOf<AWeapon> WeaponClass = GetWeaponClass(WeaponType);
	if (!WeaponClass)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateWeapon: Failed to get weapon class!"));
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AWeapon* Weapon = World->SpawnActor<AWeapon>(WeaponClass, SpawnTransform, SpawnParams);
	if (Weapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateWeapon: Successfully created weapon"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("CreateWeapon: Failed to spawn weapon actor!"));
	}

	return Weapon;
}






TSubclassOf<AWeapon> UWeaponFactory::GetWeaponClass(EWeaponType WeaponType)
{
	switch (WeaponType)
	{
	case EWeaponType::AK47:
		return AAK47::StaticClass();
	case EWeaponType::DesertEagle:
		return ADesertEagle::StaticClass();
	default:
		return nullptr;
	}
}