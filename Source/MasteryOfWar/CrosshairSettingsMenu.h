#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Components/Button.h"
// Удалена строка с ColorPicker.h
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
	UPROPERTY(meta = (BindWidget))
	class USlider* SizeSlider;

	UPROPERTY(meta = (BindWidget))
	class USlider* WidthSlider;

	UPROPERTY(meta = (BindWidget))
	class USlider* OpacitySlider;

	UPROPERTY(meta = (BindWidget))
	class USlider* DotSizeSlider;

	UPROPERTY(meta = (BindWidget))
	class UCheckBox* ShowDotCheckBox;

	UPROPERTY(meta = (BindWidget))
	class UButton* SaveButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* ResetButton;

	UPROPERTY(meta = (BindWidget))
	UCrosshair* PreviewCrosshair;

	UPROPERTY(meta = (BindWidget))
	class USlider* RedSlider;

	UPROPERTY(meta = (BindWidget))
	class USlider* GreenSlider;

	UPROPERTY(meta = (BindWidget))
	class USlider* BlueSlider;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnSizeChanged(float Value);

	UFUNCTION()
	void OnWidthChanged(float Value);

	UFUNCTION()
	void OnOpacityChanged(float Value);

	UFUNCTION()
	void OnDotSizeChanged(float Value);

	UFUNCTION()
	void OnShowDotChanged(bool bIsChecked);

	UFUNCTION()
	void OnSaveSettings();

	UFUNCTION()
	void OnResetSettings();

private:
	FCrosshairSettings CurrentSettings;
};