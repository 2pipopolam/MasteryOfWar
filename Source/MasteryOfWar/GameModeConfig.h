#pragma once

#include "CoreMinimal.h"
#include "WeaponConfig.h"
#include "GameFramework/Character.h"
#include "GameModeConfig.generated.h"

UENUM(BlueprintType)
enum class EGameModeType : uint8
{
	RifleMode    UMETA(DisplayName = "Rifle Mode"),
	PistolMode   UMETA(DisplayName = "Pistol Mode")
};

USTRUCT(BlueprintType)
struct FCharacterAnimConfig
{
	GENERATED_BODY()

	// character mesh (third person view)
	UPROPERTY(EditDefaultsOnly, Category = "Character|Animation")
	USkeletalMesh* CharacterMesh = nullptr;

	// character animation
	UPROPERTY(EditDefaultsOnly, Category = "Character|Animation")
	TSubclassOf<UAnimInstance> AnimationClass = nullptr;

	// hands mesh (first person mode)
	UPROPERTY(EditDefaultsOnly, Category = "Character|Animation")
	USkeletalMesh* ArmsMesh = nullptr;

	// class of hands animations
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
		, CharacterClass(nullptr)
	{
	}
	

	// type game mode
	UPROPERTY(EditDefaultsOnly, Category = "GameMode")
	EGameModeType ModeType;

	// weapon type for certain mode
	UPROPERTY(EditDefaultsOnly, Category = "GameMode|Weapon")
	EWeaponType WeaponType;

	// config animations
	UPROPERTY(EditDefaultsOnly, Category = "GameMode|Character")
	FCharacterAnimConfig AnimConfig;

	// path to map
	UPROPERTY(EditDefaultsOnly, Category = "GameMode|Level")
	FSoftObjectPath LevelPath;

	// character class for certain mode
	UPROPERTY(EditDefaultsOnly, Category = "GameMode|Character")
	TSubclassOf<ACharacter> CharacterClass;


	/*
	FGameModeConfig()
	{
		ModeType = EGameModeType::RifleMode;
		WeaponType = EWeaponType::AK47;
		CharacterClass = nullptr;
	}
	*/
};