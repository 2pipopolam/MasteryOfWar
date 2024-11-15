#pragma once

#include "CoreMinimal.h"
#include "WeaponSystem.h"
#include "WeaponConfig.h"
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