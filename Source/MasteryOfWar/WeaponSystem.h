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
#include "WeaponInterface.h"
#include "NetworkStructs.h"
#include "WeaponSystem.generated.h"

class UBulletFireBehavior;

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

// Camera Recoil Component
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MASTERYOFWAR_API UCameraRecoilComponent : public UActorComponent
{
    GENERATED_BODY()

public:    
    UCameraRecoilComponent();
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void ApplyRecoil();
    void ResetRecoil();
    void SetRecoilPattern(const FCameraRecoilPattern& NewPattern);
    void SetTargetCamera(UCameraComponent* NewCamera) { TargetCamera = NewCamera; }
    UCameraComponent* GetCharacterCamera() const;
    
protected:
    virtual void BeginPlay() override;

private:
    FCameraRecoilPattern CurrentPattern;
    int32 CurrentPatternIndex;
    bool bIsRecoilActive;
    FVector2D TotalRecoilOffset;
    FVector2D RecoilRecoveryOffset;
    
    float TimeSinceLastRecoil;
    bool bIsInSmoothRecovery;
    FVector2D SmoothRecoveryStartPosition;
    
    UPROPERTY()
    UCameraComponent* TargetCamera;
    
    void RecoverFromRecoil(float DeltaTime);
    void HandleSmoothRecovery(float DeltaTime);
    FVector2D GetNextPatternPoint() const;
};

// Base Weapon Class
UCLASS(Abstract, BlueprintType, Blueprintable)
class MASTERYOFWAR_API AWeapon : public AActor, public IWeaponInterface
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
    int32 GetBaseDamage() const { return WeaponConfig.DamageConfig.BaseDamage; }

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

    UFUNCTION(BlueprintCallable, Category = "Weapon|Utilities")
    virtual FTransform GetBulletSpawnTransform() const;

    // Effects
    UFUNCTION(BlueprintCallable, Category = "Weapon|Effects")
    virtual void PlayFireEffects();

    UFUNCTION(BlueprintCallable, Category = "Weapon|Effects")
    virtual void PlayReloadEffects();

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Components")
    UStaticMeshComponent* WeaponMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Components")
    UCameraRecoilComponent* CameraRecoilComponent;

    // Projectile settings
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Projectile")
    TSubclassOf<class ABullet> BulletClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Projectile")
    float ProjectileSpeed;

    // Effects
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
    UParticleSystem* MuzzleFlashTemplate;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
    UParticleSystem* MuzzleSmokeTemplate;

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

    // IWeaponInterface
    virtual const FBaseWeaponConfig& GetWeaponConfig() const override { return WeaponConfig; }
    virtual EWeaponType GetWeaponType() const override { return WeaponConfig.WeaponType; }
    virtual const FWeaponDamageConfig& GetDamageConfig() const override { return WeaponConfig.DamageConfig; }


    virtual void SimulateShot(const FNetworkShotInfo& ShotInfo);
    bool IsFiring() const { return bIsFiring; }
    bool IsReloading() const { return MagazineState.bIsReloading; }
    int32 GetCurrentAmmo() const { return MagazineState.CurrentAmmo; }

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Weapon configuration
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Config")
    FBaseWeaponConfig WeaponConfig;

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
    FVector MuzzleFlashScale;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects")
    FVector ShellEjectScale;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects")
    FVector MuzzleFlashOffset;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects")
    FVector ShellEjectOffset;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects")
    float MuzzleFlashLifetime;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects")
    FVector MuzzleSmokeScale;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects")
    float SmokeLifetime;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects")
    FVector SmokeSpawnOffset;

    // UI
    UFUNCTION(BlueprintCallable, Category = "Weapon|UI")
    virtual void CreateAmmoWidget(APlayerController* PC);

    UFUNCTION(BlueprintCallable, Category = "Weapon|UI")
    virtual void UpdateAmmoWidget();
    
private:
    UPROPERTY()
    UAmmoWidget* AmmoWidget;
};