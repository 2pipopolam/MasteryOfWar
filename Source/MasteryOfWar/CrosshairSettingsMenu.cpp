#include "CrosshairSettingsMenu.h"
#include "Kismet/GameplayStatics.h"
#include "CrosshairSaveSettings.h"
#include "GameHUD.h"

void UCrosshairSettingsMenu::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind Size and Shape Events
    if (SizeSlider)
        SizeSlider->OnValueChanged.AddDynamic(this, &UCrosshairSettingsMenu::OnSizeChanged);
    if (WidthSlider)
        WidthSlider->OnValueChanged.AddDynamic(this, &UCrosshairSettingsMenu::OnWidthChanged);
    if (OpacitySlider)
        OpacitySlider->OnValueChanged.AddDynamic(this, &UCrosshairSettingsMenu::OnOpacityChanged);
    if (DotSizeSlider)
        DotSizeSlider->OnValueChanged.AddDynamic(this, &UCrosshairSettingsMenu::OnDotSizeChanged);
    if (GapSlider)
        GapSlider->OnValueChanged.AddDynamic(this, &UCrosshairSettingsMenu::OnGapChanged);
    if (ShowDotCheckBox)
        ShowDotCheckBox->OnCheckStateChanged.AddDynamic(this, &UCrosshairSettingsMenu::OnShowDotChanged);

    // Bind Color Events
    if (RedSlider)
        RedSlider->OnValueChanged.AddDynamic(this, &UCrosshairSettingsMenu::OnRedChanged);
    if (GreenSlider)
        GreenSlider->OnValueChanged.AddDynamic(this, &UCrosshairSettingsMenu::OnGreenChanged);
    if (BlueSlider)
        BlueSlider->OnValueChanged.AddDynamic(this, &UCrosshairSettingsMenu::OnBlueChanged);

    // Bind Button Events
    if (SaveButton)
        SaveButton->OnClicked.AddDynamic(this, &UCrosshairSettingsMenu::OnSaveSettings);
    if (ResetButton)
        ResetButton->OnClicked.AddDynamic(this, &UCrosshairSettingsMenu::OnResetSettings);
    if (BackButton)
        BackButton->OnClicked.AddDynamic(this, &UCrosshairSettingsMenu::OnBackClicked);

    // Load saved settings when menu is constructed
    LoadSavedSettings();
}

void UCrosshairSettingsMenu::LoadSavedSettings()
{
    UCrosshairSaveSettings* SaveSettings = Cast<UCrosshairSaveSettings>(
        UGameplayStatics::LoadGameFromSlot("CrosshairSettings", 0));

    if (SaveSettings)
    {
        CurrentSettings = SaveSettings->SavedSettings;
    }
    else
    {
        // If no saved settings exist, initialize with defaults
        CurrentSettings = FCrosshairSettings();
    }

    // Update UI to match current settings
    UpdateSliderValues();

    // Update preview
    if (PreviewCrosshair)
    {
        PreviewCrosshair->UpdateCrosshairAppearance(CurrentSettings);
    }
}

void UCrosshairSettingsMenu::UpdateSliderValues()
{
    if (SizeSlider) SizeSlider->SetValue(CurrentSettings.CrosshairSize);
    if (WidthSlider) WidthSlider->SetValue(CurrentSettings.LineWidth);
    if (OpacitySlider) OpacitySlider->SetValue(CurrentSettings.Opacity);
    if (DotSizeSlider) DotSizeSlider->SetValue(CurrentSettings.CenterDotSize);
    if (GapSlider) GapSlider->SetValue(CurrentSettings.GapSize);
    if (ShowDotCheckBox) 
    {
        ShowDotCheckBox->SetCheckedState(
            CurrentSettings.bShowCenterDot ? ECheckBoxState::Checked : ECheckBoxState::Unchecked
        );
    }

    // Update color sliders
    if (RedSlider) RedSlider->SetValue(CurrentSettings.CrosshairColor.R);
    if (GreenSlider) GreenSlider->SetValue(CurrentSettings.CrosshairColor.G);
    if (BlueSlider) BlueSlider->SetValue(CurrentSettings.CrosshairColor.B);
}

// Size and Shape Event Handlers
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

void UCrosshairSettingsMenu::OnGapChanged(float Value)
{
    CurrentSettings.GapSize = Value;
    if (PreviewCrosshair)
        PreviewCrosshair->UpdateCrosshairAppearance(CurrentSettings);
}

void UCrosshairSettingsMenu::OnShowDotChanged(bool bIsChecked)
{
    CurrentSettings.bShowCenterDot = bIsChecked;
    if (PreviewCrosshair)
        PreviewCrosshair->UpdateCrosshairAppearance(CurrentSettings);
}

// Color Event Handlers
void UCrosshairSettingsMenu::OnRedChanged(float Value)
{
    CurrentSettings.CrosshairColor.R = Value;
    UpdateCrosshairColor();
}

void UCrosshairSettingsMenu::OnGreenChanged(float Value)
{
    CurrentSettings.CrosshairColor.G = Value;
    UpdateCrosshairColor();
}

void UCrosshairSettingsMenu::OnBlueChanged(float Value)
{
    CurrentSettings.CrosshairColor.B = Value;
    UpdateCrosshairColor();
}

void UCrosshairSettingsMenu::UpdateCrosshairColor()
{
    if (PreviewCrosshair)
        PreviewCrosshair->UpdateCrosshairAppearance(CurrentSettings);
}

void UCrosshairSettingsMenu::OnSaveSettings()
{
    UCrosshairSaveSettings* SaveSettings = Cast<UCrosshairSaveSettings>(
        UGameplayStatics::CreateSaveGameObject(UCrosshairSaveSettings::StaticClass()));

    if (SaveSettings)
    {
        SaveSettings->SavedSettings = CurrentSettings;
        if (UGameplayStatics::SaveGameToSlot(SaveSettings, "CrosshairSettings", 0))
        {
            // Apply settings to the game crosshair
            ApplySettingsToGameCrosshair();
        }
    }
}

void UCrosshairSettingsMenu::ApplySettingsToGameCrosshair()
{
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (AGameHUD* GameHUD = Cast<AGameHUD>(PC->GetHUD()))
        {
            if (UCrosshair* GameCrosshair = GameHUD->GetCrosshairWidget())
            {
                GameCrosshair->UpdateCrosshairAppearance(CurrentSettings);
            }
        }
    }
}

void UCrosshairSettingsMenu::OnResetSettings()
{
    // Reset to default settings
    CurrentSettings = FCrosshairSettings();

    // Update UI
    UpdateSliderValues();

    // Update preview
    if (PreviewCrosshair)
        PreviewCrosshair->UpdateCrosshairAppearance(CurrentSettings);
    
    // Apply to game crosshair if we're in game
    ApplySettingsToGameCrosshair();
}


void UCrosshairSettingsMenu::OnBackClicked()
{
    UGameplayStatics::OpenLevel(GetWorld(), FName("OptionsMenuMap"));
}