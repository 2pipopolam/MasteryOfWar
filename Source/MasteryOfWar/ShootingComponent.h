#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Particles/ParticleSystem.h"
#include "ShootingComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MASTERYOFWAR_API  UShootingComponent : public UActorComponent
{
	GENERATED_BODY()

public:    
	UShootingComponent();
	virtual ~UShootingComponent() override;

	UFUNCTION(BlueprintCallable, Category = "Shooting")
	void Shoot();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category = "Shooting", meta = (AllowPrivateAccess = "true"))
	float WeaponRange = 10000.0f;
    
	UPROPERTY(EditAnywhere, Category = "Shooting", meta = (AllowPrivateAccess = "true"))
	float WeaponDamage = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Effects", meta = (AllowPrivateAccess = "true"))
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, Category = "Effects", meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactEffect;
	
};