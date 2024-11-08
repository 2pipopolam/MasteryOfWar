#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "WeaponSystem.h"
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

    // Weapon configuration
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    TSubclassOf<AWeapon> DefaultWeaponClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    FName WeaponSocketName = FName("WeaponSocket");

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void AttachWeapon(AAK47* Weapon);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    bool IsDebugLineEnabled() const { return bShowDebugLine; }

private:
    bool bShowDebugLine = false;

    UFUNCTION()
    void ToggleDebugLine();

    /** First Person Arms Mesh */
    UPROPERTY(VisibleDefaultsOnly, Category = "Mesh")
    USkeletalMeshComponent* FPSArms;

    /** Preview Weapon Mesh Component */
    UPROPERTY(VisibleDefaultsOnly, Category = "Weapon")
    UStaticMeshComponent* WeaponMeshComponent;

    /** Follow camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;
    
    /** MappingContext */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputMappingContext* DefaultMappingContext;

    /** Jump Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* JumpAction;

    /** Move Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* MoveAction;

    /** Look Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* LookAction;

    /** Fire Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* FireAction;

    /** Reload Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* ReloadAction;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Weapon")
    AWeapon* CurrentWeapon;

    /** Called for movement input */
    void Move(const FInputActionValue& Value);

    /** Called for looking input */
    void Look(const FInputActionValue& Value);

    /** Called for fire input */
    void StartFire();
    void StopFire();

    /** Called for reload input */
    UFUNCTION(BlueprintCallable, Category = "Character")
    void OnReload();
    
    // APawn interface
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    
    // To add mapping context
    virtual void BeginPlay() override;

public:
    /** Returns FollowCamera subobject **/
    FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

    /** Returns FPSArms subobject **/
    FORCEINLINE class USkeletalMeshComponent* GetFPSArms() const { return FPSArms; }

    /** Returns WeaponMeshComponent subobject **/
    FORCEINLINE class UStaticMeshComponent* GetWeaponMeshComponent() const { return WeaponMeshComponent; }
};