#include "BulletFireBehavior.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "MasteryOfWarCharacter.h"

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

    
    FTransform MuzzleTransform = Weapon->GetMuzzleSocketTransform();
    
    
    if (MuzzleTransform.Equals(FTransform::Identity))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid muzzle transform - cannot spawn bullet"));
        return;
    }

    FVector SpawnLocation = MuzzleTransform.GetLocation();
    FVector Direction = Weapon->GetAdjustedAimDirection();

   
    DrawDebugSphere(
        World,
        SpawnLocation,
        5.0f,
        12,
        FColor::Green,
        false,
        5.0f
    );

    DrawDebugLine(
        World,
        SpawnLocation,
        SpawnLocation + Direction * 100.0f,
        FColor::Blue,
        false,
        5.0f,
        0,
        2.0f
    );

    TSubclassOf<ABullet> BulletClass = Weapon->GetBulletClass();
    if (!BulletClass)
    {
        UE_LOG(LogTemp, Error, TEXT("BulletClass is null!"));
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = Weapon;
    SpawnParams.Instigator = Cast<APawn>(Weapon->GetOwner());

    ABullet* Bullet = World->SpawnActor<ABullet>(
        BulletClass,
        SpawnLocation,
        Direction.Rotation(),
        SpawnParams
    );

    if (Bullet)
    {
        float Damage = FMath::RandRange(Weapon->GetMinDamage(), Weapon->GetMaxDamage());
        float Speed = 8000.0f;
        Bullet->InitializeBullet(Damage, Speed, Weapon->GetRange());
        
        UE_LOG(LogTemp, Warning, TEXT("Bullet spawned successfully from weapon at location: %s"), *SpawnLocation.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn bullet from weapon at location: %s"), *SpawnLocation.ToString());
    }
}