#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "WeaponSystem.h"
#include "GameModeConfig.h"
#include "MasteryOfWarCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AMasteryOfWarCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AMasteryOfWarCharacter();

    // Mode init
    UFUNCTION(BlueprintCallable, Category = "Character|Configuration")
    void InitializeForGameMode(const FGameModeConfig& ModeConfig);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    bool IsDebugLineEnabled() const { return bShowDebugLine; }

    // components getters
    FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
    FORCEINLINE class USkeletalMeshComponent* GetFPSArms() const { return FPSArms; }
    FORCEINLINE class UStaticMeshComponent* GetWeaponMeshComponent() const { return WeaponMeshComponent; }
    FORCEINLINE class AWeapon* GetCurrentWeapon() const { return CurrentWeapon; }

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // func for weapons
    void EquipWeaponForMode(EWeaponType WeaponType);
    void ApplyAnimationConfig(const FCharacterAnimConfig& AnimConfig);
    void SetupExistingWeapon();

    // Input functions
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void StartFire();
    void StopFire();
    void OnReload();

    UPROPERTY(BlueprintReadOnly, Category = "Weapon")
    AWeapon* CurrentWeapon;

    // Current mode config
    UPROPERTY()
    FGameModeConfig CurrentModeConfig;

private:
    bool bShowDebugLine = false;
    
    UFUNCTION()
    void ToggleDebugLine();

    // Components
    UPROPERTY(VisibleDefaultsOnly, Category = "Mesh")
    USkeletalMeshComponent* FPSArms;

    UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
    UStaticMeshComponent* WeaponMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

    // Input properties
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* FireAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* ReloadAction;
};