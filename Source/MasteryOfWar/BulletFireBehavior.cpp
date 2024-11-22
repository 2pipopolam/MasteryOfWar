#include "BulletFireBehavior.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "MasteryOfWarCharacter.h"

void UBulletFireBehavior::Fire(AWeapon* Weapon)
{
	if (!Weapon || !Weapon->GetBulletClass()) return;

	FTransform SpawnTransform = Weapon->GetBulletSpawnTransform();
    
	if (UWorld* World = GetWorld())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Weapon;
		SpawnParams.Instigator = Cast<APawn>(Weapon->GetOwner());

		if (ABullet* Bullet = World->SpawnActor<ABullet>(
			Weapon->GetBulletClass(), 
			SpawnTransform.GetLocation(),
			SpawnTransform.GetRotation().Rotator(),
			SpawnParams))
		{
			int32 Damage = Weapon->GetBaseDamage();
			Bullet->InitializeBullet(Damage, Weapon->ProjectileSpeed, Weapon->GetRange());
		}
	}

	Weapon->PlayFireEffects();
}