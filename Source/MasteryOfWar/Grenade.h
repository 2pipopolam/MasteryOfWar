#pragma once

#include "CoreMinimal.h"
#include "WeaponSystem.h"
#include "WeaponConfig.h"
#include "WeaponInterface.h"
#include "Grenade.generated.h"

UCLASS()
class MASTERYOFWAR_API AGrenade : public AWeapon
{
	GENERATED_BODY()

public:
	AGrenade();

	virtual void Fire() override;
	virtual void StartFiring() override;
	virtual void StopFiring() override;
	virtual bool CanFire() const override;

	// Override weapon interface methods
	virtual const FBaseWeaponConfig& GetWeaponConfig() const override { return GrenadeConfig; }
	virtual EWeaponType GetWeaponType() const override { return EWeaponType::Grenade; }
	virtual const FWeaponDamageConfig& GetDamageConfig() const override { return GrenadeConfig.DamageConfig; }

protected:
	virtual void BeginPlay() override;
	void RespawnGrenade();
	void SetupWeaponCollision();
	void InitializeWeaponConfig();
	void LoadWeaponAssets();

private:
	UPROPERTY()
	FGrenadeConfig GrenadeConfig;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade")
	TSubclassOf<class AGrenadeProjectile> GrenadeProjectileClass;

	//UPROPERTY(EditDefaultsOnly, Category = "Grenade|Sound")
	//USoundBase* ThrowSound;

	UPROPERTY()
	FTimerHandle RespawnTimerHandle;

	UPROPERTY()
	int32 CurrentAmmo;
	
	bool bCanThrow;
};