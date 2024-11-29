#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "WeaponConfig.h"
#include "GrenadeProjectile.generated.h"

UCLASS()
class MASTERYOFWAR_API AGrenadeProjectile : public AActor
{
	GENERATED_BODY()

public:
	AGrenadeProjectile();

	void Initialize(const FGrenadeConfig& Config);

	UFUNCTION()
	void OnGrenadeHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, 
					 UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }
	UStaticMeshComponent* GetGrenadeMesh() const { return GrenadeMesh; }

	const FGrenadeConfig& GetGrenadeConfig() const;

protected:
	virtual void BeginPlay() override;
    
	UFUNCTION()
	void Explode();
    
	void PerformExplosionDamage();
	void SpawnExplosionEffects();
	float CalculateDamageForDistance(float Distance) const;
	void GenerateExplosionRaycasts();

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* GrenadeMesh;

	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovement;

	FGrenadeConfig GrenadeConfig;
	FTimerHandle DetonationTimer;
	bool bHasExploded;

	//Physics
	UPROPERTY(EditDefaultsOnly, Category = "Physics")
	float Bounciness = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Physics")
	float Friction = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UParticleSystem* ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	USoundBase* ExplosionSound;

	
	void SetupInitialCollision();
};