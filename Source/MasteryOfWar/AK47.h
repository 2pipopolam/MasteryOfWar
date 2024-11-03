#pragma once

#include "CoreMinimal.h"
#include "WeaponSystem.h"
//#include "Blueprint/UserWidget.h"
//#include "AmmoWidget.h"
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

	UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
	FOnReloadComplete OnReloadComplete;

/*
	UPROPERTY()
	class UAmmoWidget* AmmoWidget;
*/

protected:
	virtual void BeginPlay() override;
	virtual void ReloadMagazine() override;
	virtual void SetupWeaponCollision();


	//void UpdateAmmoDisplay();
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "AK47|Animation")
	UAnimMontage* ReloadAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "AK47|Animation")
	UAnimMontage* FireAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "AK47|Effects")
	UParticleSystem* EjectedShellEffect;

	UPROPERTY(EditDefaultsOnly, Category = "AK47|Audio")
	USoundBase* EmptyMagazineSound;

	UPROPERTY(EditDefaultsOnly, Category = "AK47|Audio")
	USoundBase* ReloadSound;

	UPROPERTY(EditDefaultsOnly, Category = "AK47|Effects")
	float RecoilStrength = 5.0f;

	void ApplyRecoil();
	void PlayFireEffects();
	void PlayReloadEffects();
};