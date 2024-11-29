#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"
#include "Animation/AnimMontage.h"
#include "WeaponConfig.generated.h"

// structure for spread
USTRUCT(BlueprintType)
struct MASTERYOFWAR_API FSpreadConfig
{
    GENERATED_BODY()

    // basic spread
    UPROPERTY(EditDefaultsOnly, Category = "Spread|Base", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float BaseSpread = 0.5f;

    // spread during movement
    UPROPERTY(EditDefaultsOnly, Category = "Spread|Movement", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float CrouchMoveSpreadMin = 1.0f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Spread|Movement", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float CrouchMoveSpreadMax = 2.0f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Spread|Movement", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float WalkSpreadMin = 2.0f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Spread|Movement", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float WalkSpreadMax = 3.0f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Spread|Movement", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float RunSpreadMin = 4.0f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Spread|Movement", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float RunSpreadMax = 5.0f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Spread|Movement", meta = (ClampMin = "0.0", ClampMax = "20.0"))
    float JumpSpreadMin = 6.0f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Spread|Movement", meta = (ClampMin = "0.0", ClampMax = "20.0"))
    float JumpSpreadMax = 8.0f;

    //spread multipliers
    UPROPERTY(EditDefaultsOnly, Category = "Spread|Movement", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float SpeedSpreadMultiplier = 2.0f;

    // decrease spread while crouch
    UPROPERTY(EditDefaultsOnly, Category = "Spread|Stance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CrouchSpreadMultiplier = 0.7f;

    // spread increasers
    UPROPERTY(EditDefaultsOnly, Category = "Spread|Shooting")
    float SpreadIncreasePerShot = 0.3f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Spread|Shooting")
    float MaxSpreadIncrease = 5.0f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Spread|Shooting")
    float SpreadRecoveryRate = 3.0f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Spread|Shooting")
    float SpreadRecoveryDelay = 0.2f;
};

// recoil
USTRUCT(BlueprintType)
struct MASTERYOFWAR_API FCameraRecoilPattern
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, Category = "Recoil Pattern")
    TArray<FVector2D> PatternPoints;

    UPROPERTY(EditDefaultsOnly, Category = "Recoil Pattern")
    float RecoilStrength = 1.0f;

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
};

// mag structure
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

//damage structure
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

// fire mode enum
UENUM(BlueprintType)
enum class EFireMode : uint8
{
    Automatic        UMETA(DisplayName = "Automatic"),
    SemiAutomatic    UMETA(DisplayName = "Semi-Automatic")
};

// weapon type enum
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    AK47            UMETA(DisplayName = "AK-47"),
    DesertEagle     UMETA(DisplayName = "Desert Eagle"),
    AWP             UMETA(DisplayName = "AWP"),
    Grenade         UMETA(DisplayName = "Grenade"),
    None            UMETA(DisplayName = "None")
};

// basic config
USTRUCT(BlueprintType)
struct MASTERYOFWAR_API FBaseWeaponConfig
{
    GENERATED_BODY()

    // weapon ID
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Identity")
    EWeaponType WeaponType = EWeaponType::None;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Identity")
    EFireMode FireMode = EFireMode::SemiAutomatic;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Basic")
    float FireRate = 0.1f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Basic")
    float Range = 1000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Damage")
    FWeaponDamageConfig DamageConfig;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Spread")
    FSpreadConfig SpreadConfig;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Magazine")
    int32 MaxAmmo = 30;

    
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Reload")
    float ReloadTime = 2.0f;

    
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Recoil")
    float RecoilOffset = 3.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Recoil")
    float RecoilRotation = 2.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Recoil")
    float RecoilRecoverySpeed = 8.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Recoil")
    float RecoilRandomness = 0.3f;

    // sockets
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Sockets")
    FName MuzzleSocketName = "MuzzleSocket";

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Sockets")
    FName ShellEjectSocketName = "ShellEjectSocket";

    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Spawn")
    FVector MuzzleOffset = FVector(0.0f, 0.0f, 30.0f);
};

//AK47
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

        // DAMAGE
        DamageConfig.BaseDamage = 30;
        DamageConfig.HeadMultiplier = 4.0f;
        DamageConfig.BodyMultiplier = 1.0f;
        DamageConfig.ArmsMultiplier = 0.7f;
        DamageConfig.LegsMultiplier = 0.6f;

        //SPREAD
        SpreadConfig.BaseSpread = 0.3f;
        SpreadConfig.CrouchMoveSpreadMin = 0.35f;
        SpreadConfig.CrouchMoveSpreadMax = 0.6f;
        SpreadConfig.WalkSpreadMin = 1.0f;
        SpreadConfig.WalkSpreadMax = 1.6f;
        SpreadConfig.RunSpreadMin = 1.8f;
        SpreadConfig.RunSpreadMax = 2.2f;
        SpreadConfig.JumpSpreadMin = 4.0f;
        SpreadConfig.JumpSpreadMax = 5.0f;
        SpreadConfig.SpeedSpreadMultiplier = 1.1f;
        SpreadConfig.CrouchSpreadMultiplier = 0.7f;
        SpreadConfig.SpreadIncreasePerShot = 0.1f;
        SpreadConfig.MaxSpreadIncrease = 3.0f;
        SpreadConfig.SpreadRecoveryRate = 2.0f;
        SpreadConfig.SpreadRecoveryDelay = 0.2f;
        
        FCameraRecoilPattern Pattern;
        Pattern.RecoilStrength = 0.3f;
        Pattern.RecoverySpeed = 2.0f;
        Pattern.RandomDeviation = 0.05f;
        Pattern.RecoveryDelay = 0.5f;
        Pattern.SmoothRecoverySpeed = 2.0f;
        Pattern.MinRecoilForRecovery = 0.1f;

        // basic recoil pattern 
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

        RecoilPattern = Pattern;
    }

    /*
   // UPROPERTY(EditDefaultsOnly, Category = "AK47|Assets")
    //FSoftObjectPath ShellEjectPath = FSoftObjectPath(TEXT("/Game/Effects/Particles/P_ShellEject_AK47"));

    UPROPERTY(EditDefaultsOnly, Category = "AK47|Assets")
    FSoftObjectPath FireSoundPath = FSoftObjectPath(TEXT("/Game/Weapons/Sounds/single_shoot_ak47"));

    UPROPERTY(EditDefaultsOnly, Category = "AK47|Assets")
    FSoftObjectPath ReloadSoundPath = FSoftObjectPath(TEXT("/Game/Weapons/Sounds/reloading_ak47"));

    UPROPERTY(EditDefaultsOnly, Category = "AK47|Assets")
    FSoftObjectPath EmptyMagSoundPath = FSoftObjectPath(TEXT("/Game/Weapons/Sounds/empty_mag_sound"));

    //UPROPERTY(EditDefaultsOnly, Category = "AK47|Assets")
    //FSoftObjectPath FireAnimationPath = FSoftObjectPath(TEXT("/Game/Animations/AM_AK47_Fire"));

    //UPROPERTY(EditDefaultsOnly, Category = "AK47|Assets")
    //FSoftObjectPath ReloadAnimationPath = FSoftObjectPath(TEXT("/Game/Animations/AM_AK47_Reload"));

    UPROPERTY(EditDefaultsOnly, Category = "AK47|Recoil")
    FCameraRecoilPattern RecoilPattern;
    */

    UPROPERTY(EditDefaultsOnly, Category = "AK47|Assets")
    TObjectPtr<UParticleSystem> ShellEject;

    UPROPERTY(EditDefaultsOnly, Category = "AK47|Assets")
    TObjectPtr<USoundCue> FireSound;

    UPROPERTY(EditDefaultsOnly, Category = "AK47|Assets")
    TObjectPtr<USoundCue> ReloadSound;

    UPROPERTY(EditDefaultsOnly, Category = "AK47|Assets")
    TObjectPtr<USoundCue> EmptyMagSound;

    UPROPERTY(EditDefaultsOnly, Category = "AK47|Assets")
    TObjectPtr<UAnimMontage> FireAnimation;

    UPROPERTY(EditDefaultsOnly, Category = "AK47|Assets")
    TObjectPtr<UAnimMontage> ReloadAnimation;

    UPROPERTY(EditDefaultsOnly, Category = "AK47|Recoil")
    FCameraRecoilPattern RecoilPattern;
    
};

// Desert Eagle
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

        // DAMAGE
        DamageConfig.BaseDamage = 45;
        DamageConfig.HeadMultiplier = 4.5f;
        DamageConfig.BodyMultiplier = 1.0f;
        DamageConfig.ArmsMultiplier = 0.8f;
        DamageConfig.LegsMultiplier = 0.75f;

        // SPREAD
        SpreadConfig.BaseSpread = 0.1f;
        SpreadConfig.CrouchMoveSpreadMin = 0.3f;
        SpreadConfig.CrouchMoveSpreadMax = 0.5f;
        SpreadConfig.WalkSpreadMin = 0.8f;
        SpreadConfig.WalkSpreadMax = 1.0f;
        SpreadConfig.RunSpreadMin = 1.5f;
        SpreadConfig.RunSpreadMax = 2.0f;
        SpreadConfig.JumpSpreadMin = 4.0f;
        SpreadConfig.JumpSpreadMax = 5.0f;
        SpreadConfig.SpeedSpreadMultiplier = 1.1f;
        SpreadConfig.CrouchSpreadMultiplier = 0.75f;
        SpreadConfig.SpreadIncreasePerShot = 0.05f;
        SpreadConfig.MaxSpreadIncrease = 3.0f;
        SpreadConfig.SpreadRecoveryRate = 2.5f;
        SpreadConfig.SpreadRecoveryDelay = 0.15f;
        
        FCameraRecoilPattern Pattern;

        // strong vertical recoil
        for (int32 i = 0; i < 5; ++i)
        {
            FVector2D RecoilPoint;
            RecoilPoint.Y = 3.0f;
            RecoilPoint.X = FMath::RandRange(-2.5f, 2.5f);
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

    /*
    //UPROPERTY(EditDefaultsOnly, Category = "DesertEagle|Assets")
    //FSoftObjectPath ShellEjectPath = FSoftObjectPath(TEXT("/Game/Effects/Particles/P_ShellEject_DesertEagle"));

    UPROPERTY(EditDefaultsOnly, Category = "DesertEagle|Assets")
    FSoftObjectPath FireSoundPath = FSoftObjectPath(TEXT("/Game/Weapons/Sounds/single_shoot_deagle"));

    UPROPERTY(EditDefaultsOnly, Category = "DesertEagle|Assets")
    FSoftObjectPath ReloadSoundPath = FSoftObjectPath(TEXT("/Game/Weapons/Sounds/reloading_deagle"));

    UPROPERTY(EditDefaultsOnly, Category = "DesertEagle|Assets")
    FSoftObjectPath EmptyMagSoundPath = FSoftObjectPath(TEXT("/Game/Weapons/Sounds/empty_mag_sound"));

    //UPROPERTY(EditDefaultsOnly, Category = "DesertEagle|Assets")
    //FSoftObjectPath FireAnimationPath = FSoftObjectPath(TEXT("/Game/Animations/AM_DesertEagle_Fire"));

    //UPROPERTY(EditDefaultsOnly, Category = "DesertEagle|Assets")
    //FSoftObjectPath ReloadAnimationPath = FSoftObjectPath(TEXT("/Game/Animations/AM_DesertEagle_Reload"));

    UPROPERTY(EditDefaultsOnly, Category = "DesertEagle|Recoil")
    FCameraRecoilPattern RecoilPattern;
    */

    UPROPERTY(EditDefaultsOnly, Category = "DesertEagle|Assets")
    TObjectPtr<UParticleSystem> ShellEject;

    UPROPERTY(EditDefaultsOnly, Category = "DesertEagle|Assets")
    TObjectPtr<USoundCue> FireSound;

    UPROPERTY(EditDefaultsOnly, Category = "DesertEagle|Assets")
    TObjectPtr<USoundCue> ReloadSound;

    UPROPERTY(EditDefaultsOnly, Category = "DesertEagle|Assets")
    TObjectPtr<USoundCue> EmptyMagSound;

    UPROPERTY(EditDefaultsOnly, Category = "DesertEagle|Assets")
    TObjectPtr<UAnimMontage> FireAnimation;

    UPROPERTY(EditDefaultsOnly, Category = "DesertEagle|Assets")
    TObjectPtr<UAnimMontage> ReloadAnimation;

    UPROPERTY(EditDefaultsOnly, Category = "DesertEagle|Recoil")
    FCameraRecoilPattern RecoilPattern; 
};



USTRUCT(BlueprintType)
struct MASTERYOFWAR_API FGrenadeConfig : public FBaseWeaponConfig
{
    GENERATED_BODY()
    
    FGrenadeConfig()
    {
        WeaponType = EWeaponType::Grenade;
        FireMode = EFireMode::SemiAutomatic;

        MaxAmmo = 1; 
        
        // Throwing parameters
        ThrowForce = 3000.0f;  
        RespawnDelay = 0.9f;
        DetonationDelay = 1.1f;
        
        // Explosion settings
        ExplosionRadius = 500.0f;
        NumRaycastsPerExplosion = 1000;
        
        // Damage configuration
        DamageConfig.BaseDamage = 85; //Max damage
        DamageConfig.HeadMultiplier = 4.0f;
        DamageConfig.BodyMultiplier = 1.0f;
        DamageConfig.ArmsMultiplier = 0.8f;
        DamageConfig.LegsMultiplier = 0.75f;
        
        // Damage falloff
        DamageRadiusInner = 200.0f;  // Full damage within this radius
        DamageRadiusOuter = 500.0f;  // No damage beyond this radius
        DamageFalloffExponent = 1.0f; // Linear falloff
    }
    
    // Core settings
    UPROPERTY(EditDefaultsOnly, Category = "Grenade|Throwing")
    float ThrowForce;
    
    UPROPERTY(EditDefaultsOnly, Category = "Grenade|Gameplay")
    float RespawnDelay;
    
    UPROPERTY(EditDefaultsOnly, Category = "Grenade|Gameplay")
    float DetonationDelay;
    
    // Explosion parameters
    UPROPERTY(EditDefaultsOnly, Category = "Grenade|Explosion")
    float ExplosionRadius;
    
    UPROPERTY(EditDefaultsOnly, Category = "Grenade|Explosion")
    int32 NumRaycastsPerExplosion;
    
    UPROPERTY(EditDefaultsOnly, Category = "Grenade|Explosion")
    float DamageRadiusInner;
    
    UPROPERTY(EditDefaultsOnly, Category = "Grenade|Explosion")
    float DamageRadiusOuter;
    
    UPROPERTY(EditDefaultsOnly, Category = "Grenade|Explosion")
    float DamageFalloffExponent;
    
    // Effects
    UPROPERTY(EditDefaultsOnly, Category = "Grenade|Effects")
    TObjectPtr<UParticleSystem> ExplosionEffect;
    
    UPROPERTY(EditDefaultsOnly, Category = "Grenade|Effects")
    TObjectPtr<USoundBase> ExplosionSound;
    
    UPROPERTY(EditDefaultsOnly, Category = "Grenade|Effects")
    TObjectPtr<USoundCue> ThrowSound;
    
    UPROPERTY(EditDefaultsOnly, Category = "Grenade|Effects")
    TObjectPtr<USoundCue> BounceSound;
};