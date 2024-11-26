#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "CrosshairSettingsTypes.h"
#include "Crosshair.h"
#include "GameFramework/SaveGame.h"
#include "CrosshairSettingsMenu.generated.h"

UCLASS()
class MASTERYOFWAR_API UCrosshairSettingsMenu : public UUserWidget
{
    GENERATED_BODY()

public:
    // Crosshair Size and Shape Controls
    UPROPERTY(meta = (BindWidget))
    class USlider* SizeSlider;

    UPROPERTY(meta = (BindWidget))
    class USlider* WidthSlider;

    UPROPERTY(meta = (BindWidget))
    class USlider* OpacitySlider;

    UPROPERTY(meta = (BindWidget))
    class USlider* DotSizeSlider;

    UPROPERTY(meta = (BindWidget))
    class USlider* GapSlider;

    UPROPERTY(meta = (BindWidget))
    class UCheckBox* ShowDotCheckBox;

    // Color Controls
    UPROPERTY(meta = (BindWidget))
    class USlider* RedSlider;

    UPROPERTY(meta = (BindWidget))
    class USlider* GreenSlider;

    UPROPERTY(meta = (BindWidget))
    class USlider* BlueSlider;

    // Buttons
    UPROPERTY(meta = (BindWidget))
    class UButton* SaveButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* ResetButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* BackButton;

    // Preview Widget
    UPROPERTY(meta = (BindWidget))
    UCrosshair* PreviewCrosshair;

protected:
    virtual void NativeConstruct() override;

    // Size and Shape Event Handlers
    UFUNCTION()
    void OnSizeChanged(float Value);

    UFUNCTION()
    void OnWidthChanged(float Value);

    UFUNCTION()
    void OnOpacityChanged(float Value);

    UFUNCTION()
    void OnDotSizeChanged(float Value);

    UFUNCTION()
    void OnGapChanged(float Value);

    UFUNCTION()
    void OnShowDotChanged(bool bIsChecked);

    // Color Event Handlers
    UFUNCTION()
    void OnRedChanged(float Value);

    UFUNCTION()
    void OnGreenChanged(float Value);

    UFUNCTION()
    void OnBlueChanged(float Value);

    // Button Event Handlers
    UFUNCTION()
    void OnSaveSettings();

    UFUNCTION()
    void OnResetSettings();

    UFUNCTION()
    void OnBackClicked();

    // Utility Functions
    void LoadSavedSettings();
    void UpdateCrosshairColor();
    void UpdateSliderValues();
    void ApplySettingsToGameCrosshair();

private:
    FCrosshairSettings CurrentSettings;
};