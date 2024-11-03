#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "Camera/CameraComponent.h"
#include "WeaponSystem.generated.h"

// Структура для хранения состояния магазина
USTRUCT(BlueprintType)
struct FMagazineState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazine")
    int32 CurrentAmmo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazine")
    int32 MaxAmmo;

    FMagazineState() : CurrentAmmo(0), MaxAmmo(0) {}
};

// Интерфейс расчета урона
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

// Интерфейс поведения стрельбы
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

    // Основные функции оружия
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void Fire();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void StartFiring();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void StopFiring();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual void Reload();

    // Функции магазина
    UFUNCTION(BlueprintCallable, Category = "Weapon|Magazine")
    virtual bool CanFire() const;

    UFUNCTION(BlueprintCallable, Category = "Weapon|Magazine")
    virtual void ConsumeAmmo();

    UFUNCTION(BlueprintCallable, Category = "Weapon|Magazine")
    virtual void ReloadMagazine();

    // Точки спавна и трансформации
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual FTransform GetMuzzleTransform() const;

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual FTransform GetShellEjectTransform() const;

    // Функции точности
    UFUNCTION(BlueprintCallable, Category = "Weapon|Accuracy")
    virtual FRotator CalculateSpread() const;

    UFUNCTION(BlueprintCallable, Category = "Weapon|Accuracy")
    virtual void UpdateSpread(float DeltaTime);

    // Направление стрельбы
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    virtual FVector GetAdjustedAimDirection() const;

    // Геттеры
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    float GetFireRate() const { return FireRate; }

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    UParticleSystem* GetMuzzleFlash() const { return MuzzleFlash; }

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    USoundBase* GetFireSound() const { return FireSound; }

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    TSubclassOf<class ABullet> GetBulletClass() const { return BulletClass; }

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    float GetMinDamage() const { return MinDamage; }

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    float GetMaxDamage() const { return MaxDamage; }

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    float GetRange() const { return Range; }

    // Сеттеры для компонентов поведения
    void SetDamageCalculator(TScriptInterface<IDamageCalculator> NewCalculator) { DamageCalculator = NewCalculator; }
    void SetFireBehavior(TScriptInterface<IFireBehavior> NewBehavior) { FireBehavior = NewBehavior; }

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Компоненты оружия
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    UStaticMeshComponent* WeaponModel;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    USoundBase* FireSound;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    float Range = 10000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    float MinDamage = 20.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    float MaxDamage = 40.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    float FireRate = 0.1f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    TSubclassOf<class ABullet> BulletClass;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    UParticleSystem* MuzzleFlash;

    // Параметры точности
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Accuracy")
    float BaseSpread = 0.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Accuracy")
    float MovementSpread = 0.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Accuracy")
    float JumpingSpread = 0.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Accuracy")
    float SpreadRecoveryRate = 1.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Accuracy")
    float MaxSpread = 5.0f;

    // Сокеты
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Sockets")
    FName MuzzleSocketName = "MuzzleSocket";

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Sockets")
    FName ShellEjectSocketName = "ShellEjectSocket";

    // Смещение спавна
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Spawn")
    FVector MuzzleOffset = FVector(0.0f, 0.0f, 0.0f);

    // Состояние
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Magazine")
    FMagazineState MagazineState;

    float CurrentSpread;
    bool bIsFiring;

    // Компоненты поведения
    UPROPERTY()
    TScriptInterface<IDamageCalculator> DamageCalculator;

    UPROPERTY()
    TScriptInterface<IFireBehavior> FireBehavior;

    FTimerHandle AutoFireTimerHandle;

    // Вспомогательные функции
    bool IsCharacterMoving() const;
    bool IsCharacterJumping() const;
};