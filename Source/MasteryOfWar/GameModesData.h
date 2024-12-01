#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameModeConfig.h"
#include "GameModesData.generated.h"

UCLASS(BlueprintType, Blueprintable)
class MASTERYOFWAR_API UGameModesData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // Конфигурации всех режимов
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Modes")
    TMap<EGameModeType, FGameModeConfig> ModesConfig;

    // Получить конфигурацию для определенного режима
    UFUNCTION(BlueprintCallable, Category = "Game Modes")
    FGameModeConfig GetModeConfig(EGameModeType ModeType)
    {
        UE_LOG(LogTemp, Warning, TEXT("Getting config for mode: %s"), *UEnum::GetValueAsString(ModeType));
        
        if (const FGameModeConfig* Config = ModesConfig.Find(ModeType))
        {
            // Проверяем правильность конфигурации
            FGameModeConfig ValidConfig = *Config;
            
            // Принудительно устанавливаем правильное оружие для каждого режима
            switch(ModeType)
            {
                case EGameModeType::PistolMode:
                    if (ValidConfig.WeaponType != EWeaponType::DesertEagle)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Setting weapon type to DesertEagle for Pistol Mode"));
                        ValidConfig.WeaponType = EWeaponType::DesertEagle;
                        ValidConfig.ModeType = EGameModeType::PistolMode;
                        ValidConfig.MapType = EGameMapType::Pistol_Map;
                    }
                    break;

                case EGameModeType::RifleMode:
                    if (ValidConfig.WeaponType != EWeaponType::AK47)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Setting weapon type to AK47 for Rifle Mode"));
                        ValidConfig.WeaponType = EWeaponType::AK47;
                        ValidConfig.ModeType = EGameModeType::RifleMode;
                        ValidConfig.MapType = EGameMapType::Rifle_Map;
                    }
                    break;

                case EGameModeType::GrenadeMode:
                    if (ValidConfig.WeaponType != EWeaponType::Grenade)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Setting weapon type to Grenade for Grenade Mode"));
                        ValidConfig.WeaponType = EWeaponType::Grenade;
                        ValidConfig.ModeType = EGameModeType::GrenadeMode;
                        ValidConfig.MapType = EGameMapType::Grenade_Map;
                    }
                    break;

                default:
                    UE_LOG(LogTemp, Error, TEXT("Invalid game mode type: %s"), *UEnum::GetValueAsString(ModeType));
                    return FGameModeConfig();
            }

            UE_LOG(LogTemp, Warning, TEXT("Returning config - Mode: %s, Weapon: %s, Map: %s"),
                *UEnum::GetValueAsString(ValidConfig.ModeType),
                *UEnum::GetValueAsString(ValidConfig.WeaponType),
                *UEnum::GetValueAsString(ValidConfig.MapType));
                
            return ValidConfig;
        }

        UE_LOG(LogTemp, Error, TEXT("No configuration found for mode: %s"), *UEnum::GetValueAsString(ModeType));
        return FGameModeConfig();
    }

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override
    {
        Super::PostEditChangeProperty(PropertyChangedEvent);

        if (PropertyChangedEvent.Property)
        {
            for (auto& Pair : ModesConfig)
            {
                const EGameModeType Mode = Pair.Key;
                FGameModeConfig& Config = Pair.Value;
                
                switch(Mode)
                {
                    case EGameModeType::PistolMode:
                        Config.WeaponType = EWeaponType::DesertEagle;
                        Config.ModeType = EGameModeType::PistolMode;
                        Config.MapType = EGameMapType::Pistol_Map;
                        break;

                    case EGameModeType::RifleMode:
                        Config.WeaponType = EWeaponType::AK47;
                        Config.ModeType = EGameModeType::RifleMode;
                        Config.MapType = EGameMapType::Rifle_Map;
                        break;

                    case EGameModeType::GrenadeMode:
                        Config.WeaponType = EWeaponType::Grenade;
                        Config.ModeType = EGameModeType::GrenadeMode;
                        Config.MapType = EGameMapType::Grenade_Map;
                        break;

                    default:
                        UE_LOG(LogTemp, Error, TEXT("Invalid game mode type in editor: %s"), *UEnum::GetValueAsString(Mode));
                        break;
                }
            }
        }
    }
#endif
};