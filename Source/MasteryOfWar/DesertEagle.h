#pragma once

#include "CoreMinimal.h"
#include "WeaponSystem.h"
#include "WeaponConfig.h"
#include "WeaponInterface.h"
#include "DesertEagle.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDesertEagleReloadComplete);

UCLASS()
class MASTERYOFWAR_API ADesertEagle : public AWeapon 
{
	GENERATED_BODY()

public:
	ADesertEagle();

	virtual void Fire() override;
	virtual void StartFiring() override;
	virtual void StopFiring() override;
	virtual void Reload() override;
	virtual bool CanFire() const override;

	UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
	FOnDesertEagleReloadComplete OnReloadComplete;

	// override AWeapon's methods (with inherits IWeaponInterface)
	virtual const FBaseWeaponConfig& GetWeaponConfig() const override { return DesertEagleConfig; }
	virtual EWeaponType GetWeaponType() const override { return EWeaponType::DesertEagle; }
	virtual const FWeaponDamageConfig& GetDamageConfig() const override { return DesertEagleConfig.DamageConfig; }
    
protected:
	virtual void BeginPlay() override;
	virtual void PlayFireEffects() override;
	virtual void PlayReloadEffects() override;

private:
	void InitializeWeaponConfig();
	void LoadWeaponAssets();
	void SetupWeaponCollision();
    
	UPROPERTY()
	FDesertEagleConfig DesertEagleConfig;

	UPROPERTY(EditDefaultsOnly, Category = "DesertEagle|Effects")
	UParticleSystem* EjectedShellEffect;
};