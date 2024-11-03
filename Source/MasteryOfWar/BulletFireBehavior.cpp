#include "BulletFireBehavior.h"
#include "Kismet/GameplayStatics.h"

/*
void UBulletFireBehavior::Fire(AWeapon* Weapon)
{
	UE_LOG(LogTemp, Warning, TEXT("BulletFireBehavior::Fire called"));
    
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

	// Получаем направление стрельбы
	FVector Direction = Weapon->GetAdjustedAimDirection();
	FTransform MuzzleTransform = Weapon->GetMuzzleTransform();
    
	UE_LOG(LogTemp, Warning, TEXT("Attempting to spawn bullet at location: %s"), 
		   *MuzzleTransform.GetLocation().ToString());

	// Проверяем класс пули
	TSubclassOf<ABullet> BulletClass = Weapon->GetBulletClass();
	if (!BulletClass)
	{
		UE_LOG(LogTemp, Error, TEXT("BulletClass is null!"));
		return;
	}

	// Создаем пулю
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Weapon;
	SpawnParams.Instigator = Cast<APawn>(Weapon->GetOwner());

	ABullet* Bullet = World->SpawnActor<ABullet>(
		BulletClass,
		MuzzleTransform.GetLocation(),
		Direction.Rotation(),
		SpawnParams
	);

	if (Bullet)
	{
		UE_LOG(LogTemp, Warning, TEXT("Bullet spawned successfully"));
		float Damage = FMath::RandRange(Weapon->GetMinDamage(), Weapon->GetMaxDamage());
		Bullet->InitializeBullet(Damage, 5000.0f, Weapon->GetRange());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn bullet"));
	}
}
*/



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

    // Получаем направление стрельбы с учетом разброса
    FVector Direction = Weapon->GetAdjustedAimDirection();
    FTransform MuzzleTransform = Weapon->GetMuzzleTransform();

    // Проверяем класс пули
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
    
    
    
    
    
    
    
    

    // Создаем пулю с нужным направлением
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
        float Speed = 8000.0f; // Увеличенная скорость для лучшей видимости
        Bullet->InitializeBullet(Damage, Speed, Weapon->GetRange());
        
        // Добавляем трейл-эффект (опционально)
        if (UGameplayStatics::GetPlayerController(World, 0))
        {
            // Можно добавить трейл или другие визуальные эффекты здесь
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn bullet"));
    }
}