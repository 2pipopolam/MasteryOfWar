#pragma once

#include "CoreMinimal.h"
#include "WeaponSystem.h"
#include "WeaponConfig.h"
#include "WeaponInterface.h"
#include "AK47.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReloadComplete);

UCLASS()
class MASTERYOFWAR_API AAK47 : public AWeapon
{
	GENERATED_BODY()

public:
	AAK47();

	virtual void Fire() override;
	virtual void StartFiring() override;
	virtual void StopFiring() override;
	virtual void Reload() override;
	virtual bool CanFire() const override;

	UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
	FOnReloadComplete OnReloadComplete;

	// override AWeapon's methods (with inherits IWeaponInterface)
	virtual const FBaseWeaponConfig& GetWeaponConfig() const override { return AK47Config; }
	virtual EWeaponType GetWeaponType() const override { return EWeaponType::AK47; }
	virtual const FWeaponDamageConfig& GetDamageConfig() const override { return AK47Config.DamageConfig; }

protected:
	virtual void BeginPlay() override;
	virtual void PlayFireEffects() override;
	virtual void PlayReloadEffects() override;

private:
	void InitializeWeaponConfig();
	void LoadWeaponAssets();
	void SetupWeaponCollision();
    
	UPROPERTY()
	FAK47Config AK47Config;

	UPROPERTY(EditDefaultsOnly, Category = "AK47|Effects")
	UParticleSystem* EjectedShellEffect;
};