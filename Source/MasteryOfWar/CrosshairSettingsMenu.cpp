#include "CrosshairSettingsMenu.h"
#include "Kismet/GameplayStatics.h"

void UCrosshairSettingsMenu::NativeConstruct()
{
    Super::NativeConstruct();

    //attach events
    
    if (SizeSlider)
        SizeSlider->OnValueChanged.AddDynamic(this, &UCrosshairSettingsMenu::OnSizeChanged);
    if (WidthSlider)
        WidthSlider->OnValueChanged.AddDynamic(this, &UCrosshairSettingsMenu::OnWidthChanged);
    if (OpacitySlider)
        OpacitySlider->OnValueChanged.AddDynamic(this, &UCrosshairSettingsMenu::OnOpacityChanged);
    if (DotSizeSlider)
        DotSizeSlider->OnValueChanged.AddDynamic(this, &UCrosshairSettingsMenu::OnDotSizeChanged);
    if (ShowDotCheckBox)
        ShowDotCheckBox->OnCheckStateChanged.AddDynamic(this, &UCrosshairSettingsMenu::OnShowDotChanged);
    if (SaveButton)
        SaveButton->OnClicked.AddDynamic(this, &UCrosshairSettingsMenu::OnSaveSettings);
    if (ResetButton)
        ResetButton->OnClicked.AddDynamic(this, &UCrosshairSettingsMenu::OnResetSettings);
}

void UCrosshairSettingsMenu::OnSizeChanged(float Value)
{
    CurrentSettings.CrosshairSize = Value;
    if (PreviewCrosshair)
        PreviewCrosshair->UpdateCrosshairAppearance(CurrentSettings);
}

void UCrosshairSettingsMenu::OnWidthChanged(float Value)
{
    CurrentSettings.LineWidth = Value;
    if (PreviewCrosshair)
        PreviewCrosshair->UpdateCrosshairAppearance(CurrentSettings);
}

void UCrosshairSettingsMenu::OnOpacityChanged(float Value)
{
    CurrentSettings.Opacity = Value;
    if (PreviewCrosshair)
        PreviewCrosshair->UpdateCrosshairAppearance(CurrentSettings);
}

void UCrosshairSettingsMenu::OnDotSizeChanged(float Value)
{
    CurrentSettings.CenterDotSize = Value;
    if (PreviewCrosshair)
        PreviewCrosshair->UpdateCrosshairAppearance(CurrentSettings);
}

void UCrosshairSettingsMenu::OnShowDotChanged(bool bIsChecked)
{
    CurrentSettings.bShowCenterDot = bIsChecked;
    if (PreviewCrosshair)
        PreviewCrosshair->UpdateCrosshairAppearance(CurrentSettings);
}

void UCrosshairSettingsMenu::OnSaveSettings()
{
    // save settings implementation needed
}

void UCrosshairSettingsMenu::OnResetSettings()
{
    CurrentSettings = FCrosshairSettings();
    if (PreviewCrosshair)
        PreviewCrosshair->UpdateCrosshairAppearance(CurrentSettings);
}
