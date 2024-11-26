#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Slider.h"
#include "Components/Button.h"
#include "MasteryOfWarCharacter.h"
#include "WeaponTypes.h"
#include "ArmsPositionConfigWidget.generated.h"

UCLASS()
class MASTERYOFWAR_API UArmsPositionConfigWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

    // Position sliders
    UPROPERTY(meta = (BindWidget))
    USlider* SliderPositionX;

    UPROPERTY(meta = (BindWidget))
    USlider* SliderPositionY;

    UPROPERTY(meta = (BindWidget))
    USlider* SliderPositionZ;

    // Rotation sliders
    UPROPERTY(meta = (BindWidget))
    USlider* SliderRotationPitch;

    UPROPERTY(meta = (BindWidget))
    USlider* SliderRotationYaw;

    UPROPERTY(meta = (BindWidget))
    USlider* SliderRotationRoll;

    // Buttons
    UPROPERTY(meta = (BindWidget))
    UButton* SaveButton;

    UPROPERTY(meta = (BindWidget))
    UButton* AK47Button;

    UPROPERTY(meta = (BindWidget))
    UButton* DesertEagleButton;

    UPROPERTY(meta = (BindWidget))
    UButton* BackButton;


    // Slider event handlers
    UFUNCTION()
    void OnPositionXChanged(float Value);

    UFUNCTION()
    void OnPositionYChanged(float Value);

    UFUNCTION()
    void OnPositionZChanged(float Value);

    UFUNCTION()
    void OnRotationPitchChanged(float Value);

    UFUNCTION()
    void OnRotationYawChanged(float Value);

    UFUNCTION()
    void OnRotationRollChanged(float Value);

    // Button event handlers
    UFUNCTION()
    void OnSaveButtonClicked();

    UFUNCTION()
    void OnAK47ButtonClicked();

    UFUNCTION()
    void OnDesertEagleButtonClicked();

    
    UFUNCTION()
    void OnBackClicked();

private:
    void InitializeSliders();
    void UpdateArmsPosition();
    void SpawnCharacter(const FString& CharacterPath, EWeaponTypeEnum NewWeaponType);
    void HideCharacterMesh(AMasteryOfWarCharacter* Character);

    FVector CurrentPosition;
    FRotator CurrentRotation;
    
    // Track current weapon type
    UPROPERTY()
    EWeaponTypeEnum CurrentWeaponType;
};