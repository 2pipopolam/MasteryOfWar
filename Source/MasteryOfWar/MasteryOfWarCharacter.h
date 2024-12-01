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

    UFUNCTION(BlueprintCallable, Category = "Character|Movement")
    bool IsWalking() const { return bIsWalking; }

    // Components getters
    FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
    FORCEINLINE class USkeletalMeshComponent* GetFPSArms() const { return FPSArms; }
    FORCEINLINE class UStaticMeshComponent* GetWeaponMeshComponent() const { return WeaponMeshComponent; }
    FORCEINLINE class AWeapon* GetCurrentWeapon() const { return CurrentWeapon; }

    UFUNCTION(BlueprintCallable, Category = "Arms Configuration")
    void SetArmsPosition(const FVector& NewPosition, const FRotator& NewRotation);

    UFUNCTION(BlueprintCallable, Category = "Arms Configuration")
    static void SaveGlobalArmsPosition(const FVector& Position, const FRotator& NewRotation);

    UFUNCTION(BlueprintCallable, Category = "Arms Configuration")
    void LoadAndApplyGlobalArmsPosition();

    UFUNCTION(BlueprintCallable, Category = "Arms Configuration")
    static bool GetSavedArmsPosition(FVector& OutPosition, FRotator& OutRotation);


    void UpdateFromNetworkState(const FNetworkPlayerState& State);
    int32 GetPlayerId() const { return PlayerId; }
    void SetPlayerId(int32 NewId) { PlayerId = NewId; }

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Functions for weapons
    void EquipWeaponForMode(EWeaponType WeaponType);
    void ApplyAnimationConfig(const FCharacterAnimConfig& AnimConfig);
    void SetupExistingWeapon();

    // Input functions
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void StartFire();
    void StopFire();
    void OnReload();
    void StartCrouch();
    void StopCrouch();
    void StartWalk();
    void StopWalk();

    void OnBPressed();

    virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
    virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon")
    AWeapon* CurrentWeapon;

    // Current mode config
    UPROPERTY()
    FGameModeConfig CurrentModeConfig;

private:
    bool bShowDebugLine = false;
    bool bIsWalking = false;
    
    UFUNCTION()
    void ToggleDebugLine();

    // Components
    UPROPERTY(VisibleDefaultsOnly, Category = "Mesh")
    USkeletalMeshComponent* FPSArms;

    UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
    UStaticMeshComponent* WeaponMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

    // Movement settings
    UPROPERTY(EditDefaultsOnly, Category = "Character|Movement")
    float WalkSpeed = 200.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Character|Movement")
    float DefaultSpeed = 500.0f;

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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* CrouchAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* WalkAction;

    int32 PlayerId = -1;
    FTimerHandle NetworkUpdateTimer;
    
    void SendNetworkUpdate();
};