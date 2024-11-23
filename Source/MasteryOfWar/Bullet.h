#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/DamageType.h"
#include "TestDummy.h"
#include "Bullet.generated.h"

class UProjectileMovementComponent;
class UStaticMeshComponent;
class ATestDummy;

UCLASS()
class MASTERYOFWAR_API ABullet : public AActor
{
	GENERATED_BODY()
    
public:    
	ABullet();

	virtual void Tick(float DeltaTime) override;
    
	void InitializeBullet(float Damage, float Speed, float MaxRange);

	UFUNCTION()
	void OnBulletHit(UPrimitiveComponent* HitComp, AActor* OtherActor, 
					 UPrimitiveComponent* OtherComp, FVector NormalImpulse, 
					 const FHitResult& Hit);


	/*
	UFUNCTION()
	void OnBulletOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
						 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
						 bool bFromSweep, const FHitResult& SweepResult);
	*/


protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BulletMesh;

	UPROPERTY()
	int32 WeaponDamage;
	
	UPROPERTY()
	float MaxTravelDistance;

	FVector StartLocation;
};