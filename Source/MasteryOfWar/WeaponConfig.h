#pragma once

#include "CoreMinimal.h"
#include "WeaponConfig.generated.h"

// Recoil pattern structure
USTRUCT(BlueprintType)
struct MASTERYOFWAR_API FCameraRecoilPattern
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, Category = "Recoil Pattern")
    TArray<FVector2D> PatternPoints;

    UPROPERTY(EditDefaultsOnly, Category = "Recoil Pattern")
    float RecoilStrength = 10.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Recoil Pattern")
    float RecoverySpeed = 5.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Recoil Pattern")
    float RandomDeviation = 0.1f;

    UPROPERTY(EditDefaultsOnly, Category = "Recoil Pattern")
    float RecoveryDelay = 0.5f;

    UPROPERTY(EditDefaultsOnly, Category = "Recoil Pattern")
    float SmoothRecoverySpeed = 2.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Recoil Pattern")
    float MinRecoilForRecovery = 0.1f;

    FCameraRecoilPattern()
        : RecoilStrength(1.0f)
        , RecoverySpeed(5.0f)
        , RandomDeviation(0.1f)
        , RecoveryDelay(0.5f)
        , SmoothRecoverySpeed(2.0f)
        , MinRecoilForRecovery(0.1f)
    {
    }
};

// Magazine state structure
USTRUCT(BlueprintType)
struct MASTERYOFWAR_API FMagazineState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazine")
    int32 CurrentAmmo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazine")
    int32 MaxAmmo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazine")
    bool bIsReloading;

    FMagazineState() : CurrentAmmo(0), MaxAmmo(0), bIsReloading(false) {}
};

// Damage configuration structure
USTRUCT(BlueprintType)
struct MASTERYOFWAR_API FWeaponDamageConfig
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, Category = "Damage")
    int32 BaseDamage = 30;

    UPROPERTY(EditDefaultsOnly, Category = "Damage|Multipliers")
    float HeadMultiplier = 4.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Damage|Multipliers")
    float BodyMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Damage|Multipliers")
    float ArmsMultiplier = 0.75f;

    UPROPERTY(EditDefaultsOnly, Category = "Damage|Multipliers")
    float LegsMultiplier = 0.5f;
};

UENUM(BlueprintType)
enum class EFireMode : uint8
{
    Automatic        UMETA(DisplayName = "Automatic"),
    SemiAutomatic    UMETA(DisplayName = "Semi-Automatic")
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    AK47            UMETA(DisplayName = "AK-47"),
    DesertEagle     UMETA(DisplayName = "Desert Eagle"),
    AWP             UMETA(DisplayName = "AWP"),
    None            UMETA(DisplayName = "None")
};

// Base weapon config
USTRUCT(BlueprintType)
struct MASTERYOFWAR_API FBaseWeaponConfig
{
    GENERATED_BODY()

    // Weapon identification
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Identity")
    EWeaponType WeaponType = EWeaponType::None;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Identity")
    EFireMode FireMode = EFireMode::SemiAutomatic;

    // Basic parameters
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Basic")
    float FireRate = 0.1f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Basic")
    float Range = 1000.0f;

    // Damage configuration
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Damage")
    FWeaponDamageConfig DamageConfig;

    // Magazine configuration
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Magazine")
    int32 MaxAmmo = 30;

    // Reload configuration
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Reload")
    float ReloadTime = 2.0f;

    // Accuracy parameters
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

    // Recoil parameters
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Recoil")
    float RecoilOffset = 3.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Recoil")
    float RecoilRotation = 2.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Recoil")
    float RecoilRecoverySpeed = 8.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Recoil")
    float RecoilRandomness = 0.3f;

    // Socket names
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Sockets")
    FName MuzzleSocketName = "MuzzleSocket";

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Sockets")
    FName ShellEjectSocketName = "ShellEjectSocket";

    // Spawn configuration
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Spawn")
    FVector MuzzleOffset = FVector(0.0f, 0.0f, 30.0f);
};

// AK47-specific configuration
USTRUCT(BlueprintType)
struct MASTERYOFWAR_API FAK47Config : public FBaseWeaponConfig
{
    GENERATED_BODY()
    
    FAK47Config()
    {
        WeaponType = EWeaponType::AK47;
        FireMode = EFireMode::Automatic;
        FireRate = 0.1f;
        Range = 8000.0f;
        MaxAmmo = 30;
        ReloadTime = 2.0f;

        // Настройка урона для AK47
        DamageConfig.BaseDamage = 35;
        DamageConfig.HeadMultiplier = 4.0f;
        DamageConfig.BodyMultiplier = 1.0f;
        DamageConfig.ArmsMultiplier = 0.75f;
        DamageConfig.LegsMultiplier = 0.5f;
        
        FCameraRecoilPattern Pattern;

        // main recoil pattern for AK-47
        for (int32 i = 0; i < 10; ++i)
        {
            float progress = static_cast<float>(i) / 10.0f;
            
            FVector2D RecoilPoint;
            if (i < 5)
            {
                RecoilPoint.Y = FMath::Lerp(1.0f, 1.5f, progress);
                RecoilPoint.X = -0.2f;
            }
            else
            {
                RecoilPoint.Y = FMath::Lerp(1.5f, 2.0f, progress);
                RecoilPoint.X = 0.3f;
            }
            Pattern.PatternPoints.Add(RecoilPoint);
        }

        // horizontal recoil
        for (int32 i = 0; i < 20; ++i)
        {
            float progress = static_cast<float>(i) / 20.0f;
            
            FVector2D RecoilPoint;
            RecoilPoint.Y = 2.0f;
            RecoilPoint.X = FMath::Sin(progress * PI * 2) * 0.4f;
            
            Pattern.PatternPoints.Add(RecoilPoint);
        }

        Pattern.RecoilStrength = 0.3f;
        Pattern.RecoverySpeed = 2.0f;
        Pattern.RandomDeviation = 0.05f;
        Pattern.RecoveryDelay = 0.5f;
        Pattern.SmoothRecoverySpeed = 2.0f;
        Pattern.MinRecoilForRecovery = 0.1f;

        RecoilPattern = Pattern;
    }

    UPROPERTY(EditDefaultsOnly, Category = "AK47|Assets")
    FSoftObjectPath ShellEjectPath = FSoftObjectPath(TEXT("/Game/Effects/Particles/P_ShellEject_AK47"));

    UPROPERTY(EditDefaultsOnly, Category = "AK47|Assets")
    FSoftObjectPath FireSoundPath = FSoftObjectPath(TEXT("/Game/Weapons/Sounds/single_shoot_ak47"));

    UPROPERTY(EditDefaultsOnly, Category = "AK47|Assets")
    FSoftObjectPath ReloadSoundPath = FSoftObjectPath(TEXT("/Game/Weapons/Sounds/reloading_ak47"));

    UPROPERTY(EditDefaultsOnly, Category = "AK47|Assets")
    FSoftObjectPath EmptyMagSoundPath = FSoftObjectPath(TEXT("/Game/Weapons/Sounds/empty_mag_sound"));

    UPROPERTY(EditDefaultsOnly, Category = "AK47|Assets")
    FSoftObjectPath FireAnimationPath = FSoftObjectPath(TEXT("/Game/Animations/AM_AK47_Fire"));

    UPROPERTY(EditDefaultsOnly, Category = "AK47|Assets")
    FSoftObjectPath ReloadAnimationPath = FSoftObjectPath(TEXT("/Game/Animations/AM_AK47_Reload"));

    UPROPERTY(EditDefaultsOnly, Category = "AK47|Recoil")
    FCameraRecoilPattern RecoilPattern;
};

// Desert Eagle specific configuration
USTRUCT(BlueprintType)
struct MASTERYOFWAR_API FDesertEagleConfig : public FBaseWeaponConfig
{
    GENERATED_BODY()
    
    FDesertEagleConfig()
    {
        WeaponType = EWeaponType::DesertEagle;
        FireMode = EFireMode::SemiAutomatic;
        FireRate = 0.2f;
        Range = 5000.0f;
        MaxAmmo = 7;
        ReloadTime = 1.5f;

        // Настройка урона для Desert Eagle
        DamageConfig.BaseDamage = 55;
        DamageConfig.HeadMultiplier = 4.5f;
        DamageConfig.BodyMultiplier = 1.0f;
        DamageConfig.ArmsMultiplier = 0.8f;
        DamageConfig.LegsMultiplier = 0.6f;

        FCameraRecoilPattern Pattern;

        for (int32 i = 0; i < 5; ++i)
        {
            FVector2D RecoilPoint;
            RecoilPoint.Y = 3.0f;
            RecoilPoint.X = FMath::RandRange(-0.5f, 0.5f);
            Pattern.PatternPoints.Add(RecoilPoint);
        }

        Pattern.RecoilStrength = 0.6f;
        Pattern.RecoverySpeed = 1.5f;
        Pattern.RandomDeviation = 0.1f;
        Pattern.RecoveryDelay = 0.3f;
        Pattern.SmoothRecoverySpeed = 1.5f;
        Pattern.MinRecoilForRecovery = 0.15f;

        RecoilPattern = Pattern;
    }

    UPROPERTY(EditDefaultsOnly, Category = "DesertEagle|Assets")
    FSoftObjectPath ShellEjectPath = FSoftObjectPath(TEXT("/Game/Effects/Particles/P_ShellEject_DesertEagle"));

    UPROPERTY(EditDefaultsOnly, Category = "DesertEagle|Assets")
    FSoftObjectPath FireSoundPath = FSoftObjectPath(TEXT("/Game/Weapons/Sounds/single_shoot_deagle"));

    UPROPERTY(EditDefaultsOnly, Category = "DesertEagle|Assets")
    FSoftObjectPath ReloadSoundPath = FSoftObjectPath(TEXT("/Game/Weapons/Sounds/reloading_deagle"));

    UPROPERTY(EditDefaultsOnly, Category = "DesertEagle|Assets")
    FSoftObjectPath EmptyMagSoundPath = FSoftObjectPath(TEXT("/Game/Weapons/Sounds/empty_mag_sound"));

    UPROPERTY(EditDefaultsOnly, Category = "DesertEagle|Assets")
    FSoftObjectPath FireAnimationPath = FSoftObjectPath(TEXT("/Game/Animations/AM_DesertEagle_Fire"));

    UPROPERTY(EditDefaultsOnly, Category = "DesertEagle|Assets")
    FSoftObjectPath ReloadAnimationPath = FSoftObjectPath(TEXT("/Game/Animations/AM_DesertEagle_Reload"));

    UPROPERTY(EditDefaultsOnly, Category = "DesertEagle|Recoil")
    FCameraRecoilPattern RecoilPattern;
};