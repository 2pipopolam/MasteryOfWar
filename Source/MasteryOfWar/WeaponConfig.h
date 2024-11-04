#pragma once

#include "CoreMinimal.h"
#include "WeaponConfig.generated.h"

// Magazine state structure
USTRUCT(BlueprintType)
struct MASTERYOFWAR_API FMagazineState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazine")
    int32 CurrentAmmo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magazine")
    int32 MaxAmmo;

    FMagazineState() : CurrentAmmo(0), MaxAmmo(0) {}
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

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Basic")
    float MinDamage = 20.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Basic")
    float MaxDamage = 40.0f;

    // Magazine configuration
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Magazine")
    int32 MaxAmmo = 30;

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

    // Socket names
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Sockets")
    FName MuzzleSocketName = "MuzzleSocket";

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Sockets")
    FName ShellEjectSocketName = "ShellEjectSocket";

    // Spawn configuration
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Spawn")
    FVector MuzzleOffset = FVector(0.0f, 0.0f, 0.0f);
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
        FireRate = 0.1f;         // 600 RPM
        MinDamage = 25.0f;
        MaxDamage = 45.0f;
        Range = 8000.0f;
        MaxAmmo = 30;
        
        BaseSpread = 0.2f;
        MovementSpread = 1.5f;
        JumpingSpread = 3.0f;
        SpreadRecoveryRate = 0.5f;
        MaxSpread = 4.0f;
    }

    UPROPERTY(EditDefaultsOnly, Category = "AK47|Effects")
    float RecoilStrength = 5.0f;

    // Asset paths
    UPROPERTY(EditDefaultsOnly, Category = "AK47|Assets")
    FSoftObjectPath WeaponMeshPath = FSoftObjectPath(TEXT("/Game/Weapons/Meshes/AK47_Mesh"));

    UPROPERTY(EditDefaultsOnly, Category = "AK47|Assets")
    FSoftObjectPath MuzzleFlashPath = FSoftObjectPath(TEXT("/Game/Effects/Particles/P_MuzzleFlash_AK47"));

    UPROPERTY(EditDefaultsOnly, Category = "AK47|Assets")
    FSoftObjectPath ShellEjectPath = FSoftObjectPath(TEXT("/Game/Effects/Particles/P_ShellEject_AK47"));

    UPROPERTY(EditDefaultsOnly, Category = "AK47|Assets")
    FSoftObjectPath FireSoundPath = FSoftObjectPath(TEXT("/Game/Weapons/Sounds/single_shoot_ak47"));

    UPROPERTY(EditDefaultsOnly, Category = "AK47|Assets")
    FSoftObjectPath ReloadSoundPath = FSoftObjectPath(TEXT("/Game/Weapons/Sounds/reloading_ak47"));

    UPROPERTY(EditDefaultsOnly, Category = "AK47|Assets")
    FSoftObjectPath EmptyMagSoundPath = FSoftObjectPath(TEXT("/Game/Sounds/Weapons/S_EmptyMag"));

    UPROPERTY(EditDefaultsOnly, Category = "AK47|Assets")
    FSoftObjectPath FireAnimationPath = FSoftObjectPath(TEXT("/Game/Animations/AM_AK47_Fire"));

    UPROPERTY(EditDefaultsOnly, Category = "AK47|Assets")
    FSoftObjectPath ReloadAnimationPath = FSoftObjectPath(TEXT("/Game/Animations/AM_AK47_Reload"));
};