#pragma once

#include "CoreMinimal.h"
#include "WeaponConfig.h"
#include "GameFramework/Character.h"
#include "DamageConfig.h"
#include "GameModeConfig.generated.h"

UENUM(BlueprintType)
enum class EGameMapType : uint8
{
    Pistol_Map    UMETA(DisplayName = "Pistol Map"),
    Rifle_Map     UMETA(DisplayName = "Rifle Map"),
    Grenade_Map   UMETA(DisplayName = "Grenade Map"),
    None          UMETA(DisplayName = "None")
};

UENUM(BlueprintType)
enum class EGameModeType : uint8
{
    RifleMode    UMETA(DisplayName = "Rifle Mode"),
    PistolMode   UMETA(DisplayName = "Pistol Mode"),
    GrenadeMode  UMETA(DisplayName = "Grenade Mode"),
    None         UMETA(DisplayName = "None")
};

USTRUCT(BlueprintType)
struct FCharacterAnimConfig
{
    GENERATED_BODY()

    // Character mesh (third person view)
    UPROPERTY(EditDefaultsOnly, Category = "Character|Animation")
    USkeletalMesh* CharacterMesh = nullptr;

    // Character animation
    UPROPERTY(EditDefaultsOnly, Category = "Character|Animation")
    TSubclassOf<UAnimInstance> AnimationClass = nullptr;

    // Hands mesh (first person mode)
    UPROPERTY(EditDefaultsOnly, Category = "Character|Animation")
    USkeletalMesh* ArmsMesh = nullptr;

    // Class of hands animations
    UPROPERTY(EditDefaultsOnly, Category = "Character|Animation")
    TSubclassOf<UAnimInstance> ArmsAnimClass = nullptr;
};

USTRUCT(BlueprintType)
struct MASTERYOFWAR_API FGameModeConfig
{
    GENERATED_BODY()

    FGameModeConfig()
        : ModeType(EGameModeType::RifleMode)
        , WeaponType(EWeaponType::AK47)
        , MapType(EGameMapType::Rifle_Map)
        , CharacterClass(nullptr)
    {
    }

    // Game mode type
    UPROPERTY(EditDefaultsOnly, Category = "GameMode")
    EGameModeType ModeType;

    // Weapon type for current mode
    UPROPERTY(EditDefaultsOnly, Category = "GameMode|Weapon")
    EWeaponType WeaponType;

    // Map type for current mode
    UPROPERTY(EditDefaultsOnly, Category = "GameMode|Map")
    EGameMapType MapType;

    // Animation configuration
    UPROPERTY(EditDefaultsOnly, Category = "GameMode|Character")
    FCharacterAnimConfig AnimConfig;

    // Path to map
    UPROPERTY(EditDefaultsOnly, Category = "GameMode|Level")
    FSoftObjectPath LevelPath;

    // Character class for current mode
    UPROPERTY(EditDefaultsOnly, Category = "GameMode|Character")
    TSubclassOf<ACharacter> CharacterClass;

    // Damage configuration
    UPROPERTY(EditDefaultsOnly, Category = "GameMode|Damage")
    FModeDamageConfig DamageConfig;
};