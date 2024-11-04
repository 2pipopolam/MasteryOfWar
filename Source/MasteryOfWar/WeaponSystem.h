#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "Camera/CameraComponent.h"
#include "WeaponConfig.h"
#include "AmmoWidget.h"
#include "WeaponSystem.generated.h"

// Interfaces remain the same
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

UCLASS(Abstract)
class MASTERYOFWAR_API AWeapon : public AActor
{
    GENERATED_BODY()

public:
    AWeapon();
    virtual ~AWeapon();

    // Initialize weapon with config
    virtual void Initialize(const FBaseWeaponConfig& InConfig);

    // Base weapon functions
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void Fire();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void ProcessFireInput(bool bPressed);

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void StartFiring();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void StopFiring();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void Reload();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    TSubclassOf<class ABullet> GetBulletClass() const { return BulletClass; }
    
    // Magazine functions
    UFUNCTION(BlueprintCallable, Category = "Weapon|Magazine")
    virtual bool CanFire() const;

    UFUNCTION(BlueprintCallable, Category = "Weapon|Magazine")
    virtual void ConsumeAmmo();

    UFUNCTION(BlueprintCallable, Category = "Weapon|Magazine")
    virtual void ReloadMagazine();

    // Transform functions
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual FTransform GetMuzzleTransform() const;

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual FTransform GetShellEjectTransform() const;

    // Accuracy functions
    UFUNCTION(BlueprintCallable, Category = "Weapon|Accuracy")
    virtual FRotator CalculateSpread() const;

    UFUNCTION(BlueprintCallable, Category = "Weapon|Accuracy")
    virtual void UpdateSpread(float DeltaTime);

    // Aim functions
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual FVector GetAdjustedAimDirection() const;

    // Getters
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    float GetFireRate() const { return Config.FireRate; }

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    float GetMinDamage() const { return Config.MinDamage; }

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    float GetMaxDamage() const { return Config.MaxDamage; }

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    float GetRange() const { return Config.Range; }

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    EFireMode GetFireMode() const { return Config.FireMode; }

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    EWeaponType GetWeaponType() const { return Config.WeaponType; }


    // UI Functions
    UFUNCTION(BlueprintCallable, Category = "Weapon|UI")
    virtual void CreateAmmoWidget(APlayerController* PC);

    UFUNCTION(BlueprintCallable, Category = "Weapon|UI")
    virtual void UpdateAmmoWidget();
    
    
    // Getters for AmmoWidget
    UFUNCTION(BlueprintCallable, Category = "Weapon|Magazine")
    int32 GetCurrentAmmo() const { return MagazineState.CurrentAmmo; }

    UFUNCTION(BlueprintCallable, Category = "Weapon|Magazine")
    int32 GetMaxAmmo() const { return MagazineState.MaxAmmo; } 


    // Behavior setters
    void SetDamageCalculator(TScriptInterface<IDamageCalculator> NewCalculator) { DamageCalculator = NewCalculator; }
    void SetFireBehavior(TScriptInterface<IFireBehavior> NewBehavior) { FireBehavior = NewBehavior; }

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void HandleFireMode();

    // Configuration
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Config")
    FBaseWeaponConfig Config;

    // Components
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    UStaticMeshComponent* WeaponModel;

    // Visual and audio effects
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects")
    UParticleSystem* MuzzleFlash;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Effects")
    USoundBase* FireSound;

    // Bullet class
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    TSubclassOf<class ABullet> BulletClass;

    // State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Magazine")
    FMagazineState MagazineState;

    float CurrentSpread;
    bool bIsFiring;

    // Behaviors
    UPROPERTY()
    TScriptInterface<IDamageCalculator> DamageCalculator;

    UPROPERTY()
    TScriptInterface<IFireBehavior> FireBehavior;

    FTimerHandle AutoFireTimerHandle;

    // UI Components
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|UI")
    TSubclassOf<UAmmoWidget> AmmoWidgetClass;

    UPROPERTY()
    UAmmoWidget* AmmoWidget;


    // Helper functions
    bool IsCharacterMoving() const;
    bool IsCharacterJumping() const;
    bool IsAutomaticFireMode() const;
};