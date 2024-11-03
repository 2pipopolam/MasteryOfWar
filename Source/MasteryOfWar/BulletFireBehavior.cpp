#include "BulletFireBehavior.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "MasteryOfWarCharacter.h"

class AMasteryOfWarCharacter;

void UBulletFireBehavior::Fire(AWeapon* Weapon)
{
    if (!Weapon)
    {
        UE_LOG(LogTemp, Error, TEXT("Weapon is null in BulletFireBehavior"));
        return;
    }

    UWorld* World = Weapon->GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("World is null in BulletFireBehavior"));
        return;
    }

	// new aim dir because of recoil and etc.
    FVector Direction = Weapon->GetAdjustedAimDirection();
    FTransform MuzzleTransform = Weapon->GetMuzzleTransform();

	//check for bullet
    TSubclassOf<ABullet> BulletClass = Weapon->GetBulletClass();
    if (!BulletClass)
    {
        UE_LOG(LogTemp, Error, TEXT("BulletClass is null!"));
        return;
    }
   
	
    
	if (APawn* OwnerPawn = Cast<APawn>(Weapon->GetOwner()))
	{
		if (AMasteryOfWarCharacter* Character = Cast<AMasteryOfWarCharacter>(OwnerPawn))
		{
			if (Character->IsDebugLineEnabled()) // добавьте этот геттер в Character
			{
				DrawDebugLine(
					World,
					MuzzleTransform.GetLocation(),
					MuzzleTransform.GetLocation() + Direction * 1000.0f,
					FColor::Green,
					false,
					5.0f,
					0,
					2.0f
				);
			}
		}
	}
    
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = Weapon;
    SpawnParams.Instigator = Cast<APawn>(Weapon->GetOwner());

    FRotator BulletRotation = Direction.Rotation();
    
    ABullet* Bullet = World->SpawnActor<ABullet>(
        BulletClass,
        MuzzleTransform.GetLocation(),
        BulletRotation,
        SpawnParams
    );

    if (Bullet)
    {
        UE_LOG(LogTemp, Warning, TEXT("Bullet spawned with direction: %s"), *Direction.ToString());
        float Damage = FMath::RandRange(Weapon->GetMinDamage(), Weapon->GetMaxDamage());
        float Speed = 8000.0f;
        Bullet->InitializeBullet(Damage, Speed, Weapon->GetRange());
        
        /*
        if (UGameplayStatics::GetPlayerController(World, 0))
        {
            // some effect will be
        }
         */
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn bullet"));
    }
}