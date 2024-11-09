#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "Camera/CameraComponent.h"
#include "WeaponConfig.h"
#include "AmmoWidget.h"
#include "Bullet.h"
#include "Particles/ParticleSystemComponent.h"
#include "WeaponSystem.generated.h"

// Damage Calculator Interface
UINTERFACE(MinimalAPI)
class UDamageCalculator : public UInterface
{
    GENERATED_BODY()
};

class IDamageCalculator
{
    GENERATED_BODY()
public:
    virtual float CalculateDamage(float Distance) = 0;
};

// Fire Behavior Interface
UINTERFACE(MinimalAPI)
class UFireBehavior : public UInterface
{
    GENERATED_BODY()
};

class IFireBehavior
{
    GENERATED_BODY()
public:
    virtual void Fire(class AWeapon* Weapon) = 0;
};

// Base Weapon Class
UCLASS(Abstract, BlueprintType, Blueprintable)
class MASTERYOFWAR_API AWeapon : public AActor
{
    GENERATED_BODY()

public:
    AWeapon();

    // Core weapon functions
    UFUNCTION(BlueprintCallable, Category = "Weapon|Actions")
    virtual void Fire();

    UFUNCTION(BlueprintCallable, Category = "Weapon|Actions")
    virtual void StartFiring();

    UFUNCTION(BlueprintCallable, Category = "Weapon|Actions")
    virtual void StopFiring();

    UFUNCTION(BlueprintCallable, Category = "Weapon|Actions")
    virtual void Reload();

    UFUNCTION(BlueprintCallable, Category = "Weapon|Actions")
    virtual bool CanFire() const;

    // Getters
    UFUNCTION(BlueprintPure, Category = "Weapon")
    TSubclassOf<class ABullet> GetBulletClass() const { return BulletClass; }

    UFUNCTION(BlueprintPure, Category = "Weapon")
    float GetMinDamage() const { return WeaponConfig.MinDamage; }

    UFUNCTION(BlueprintPure, Category = "Weapon")
    float GetMaxDamage() const { return WeaponConfig.MaxDamage; }

    UFUNCTION(BlueprintPure, Category = "Weapon")
    float GetRange() const { return WeaponConfig.Range; }

    UFUNCTION(BlueprintPure, Category = "Weapon")
    EFireMode GetFireMode() const { return WeaponConfig.FireMode; }

    // Socket and aiming utilities
    UFUNCTION(BlueprintCallable, Category = "Weapon|Utilities")
    virtual FTransform GetMuzzleSocketTransform() const;

    UFUNCTION(BlueprintCallable, Category = "Weapon|Utilities")
    virtual FTransform GetShellEjectSocketTransform() const;

    UFUNCTION(BlueprintCallable, Category = "Weapon|Utilities")
    virtual FVector GetAdjustedAimDirection() const;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Components")
    UStaticMeshComponent* WeaponMesh;

    // Projectile settings
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Projectile")
    TSubclassOf<class ABullet> BulletClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Projectile")
    float ProjectileSpeed;

    // Effects
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
    UParticleSystem* MuzzleFlashTemplate;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
    UParticleSystem* ShellEjectTemplate;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
    USoundBase* FireSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
    USoundBase* EmptyMagazineSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
    USoundBase* ReloadSound;

    // Animations
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
    UAnimMontage* FireAnimation;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
    UAnimMontage* ReloadAnimation;

    // Magazine state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Magazine")
    FMagazineState MagazineState;

    // UI
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|UI")
    TSubclassOf<UAmmoWidget> AmmoWidgetClass;

    // Fire behavior
    UFUNCTION(BlueprintCallable, Category = "Weapon|Behavior")
    void SetWeaponFireBehavior(const TScriptInterface<IFireBehavior>& NewBehavior) { FireBehavior = NewBehavior; }

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Weapon configuration
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Config")
    FBaseWeaponConfig WeaponConfig;

    // Effects
    UFUNCTION(BlueprintCallable, Category = "Weapon|Effects")
    virtual void PlayFireEffects();

    UFUNCTION(BlueprintCallable, Category = "Weapon|Effects")
    virtual void PlayReloadEffects();

    // Recoil
    UFUNCTION()
    virtual void HandleRecoil();
    
    UFUNCTION()
    virtual void UpdateRecoilState(float DeltaTime);

    // Recoil state
    FVector InitialWeaponLocation;
    FRotator InitialWeaponRotation;
    FVector CurrentRecoilOffset;
    FRotator CurrentRecoilRotation;
    bool bIsInRecoil;
    float RecoilTime;

    // Utility functions
    UFUNCTION()
    virtual bool HasValidMuzzleSocket() const;

    UFUNCTION()
    virtual void UpdateSpread(float DeltaTime);

    UFUNCTION()
    virtual bool IsCharacterMoving() const;

    UFUNCTION()
    virtual bool IsCharacterJumping() const;

    UFUNCTION()
    virtual void ConsumeAmmo();

    UFUNCTION()
    virtual bool IsAutomaticFireMode() const;

    UFUNCTION()
    virtual void HandleAutoFire();

    // Effects parameters
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects")
    FVector MuzzleFlashScale = FVector(0.05f);

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects")
    FVector ShellEjectScale = FVector(0.3f);

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects")
    FVector MuzzleFlashOffset = FVector(0.0f);

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects")
    FVector ShellEjectOffset = FVector(10.0f, 5.0f, 0.0f);

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects")
    float MuzzleFlashLifetime = 0.2f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
    UParticleSystem* MuzzleSmokeTemplate;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects")
    FVector MuzzleSmokeScale = FVector(1.0f);

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects")
    float SmokeLifetime = 1.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects")
    FVector SmokeSpawnOffset = FVector(0.0f);

    // UI
    UFUNCTION(BlueprintCallable, Category = "Weapon|UI")
    virtual void CreateAmmoWidget(APlayerController* PC);

    UFUNCTION(BlueprintCallable, Category = "Weapon|UI")
    virtual void UpdateAmmoWidget();

    // Accuracy
    UFUNCTION(BlueprintCallable, Category = "Weapon|Accuracy")
    virtual FRotator CalculateSpread() const;

    // Fire behavior
    UPROPERTY()
    TScriptInterface<IFireBehavior> FireBehavior;

    // Damage calculator
    UPROPERTY()
    TScriptInterface<IDamageCalculator> DamageCalculator;

    // Weapon state
    bool bIsFiring;
    float CurrentSpread;
    FTimerHandle AutoFireTimerHandle;

private:
    UPROPERTY()
    UAmmoWidget* AmmoWidget;
};