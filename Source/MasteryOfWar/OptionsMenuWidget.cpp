#include "OptionsMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UOptionsMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // on click button functions
    if (BackButton)
    {
        BackButton->OnClicked.AddDynamic(this, &UOptionsMenuWidget::OnBackClicked);
    }
    if (CrosshairButton)
    {
        CrosshairButton->OnClicked.AddDynamic(this, &UOptionsMenuWidget::OnCrosshairClicked);
    }
    if (ViewmodelButton)
    {
        ViewmodelButton->OnClicked.AddDynamic(this, &UOptionsMenuWidget::OnViewmodelClicked);
    }
    if (VideoButton)
    {
        VideoButton->OnClicked.AddDynamic(this, &UOptionsMenuWidget::OnVideoClicked);
    }
    if (SoundButton)
    {
        SoundButton->OnClicked.AddDynamic(this, &UOptionsMenuWidget::OnSoundClicked);
    }
    if (InputButton)
    {
        InputButton->OnClicked.AddDynamic(this, &UOptionsMenuWidget::OnInputClicked);
    }
    if (MultiplayerButton)
    {
        //MultiplayerButton->OnClicked.AddDynamic(this, &UOptionsMenuWidget::OnMultiplayerClicked);
    }
}

void UOptionsMenuWidget::OnBackClicked()
{
    UGameplayStatics::OpenLevel(GetWorld(), FName("MainMenuMap"));
}

void UOptionsMenuWidget::OnCrosshairClicked()
{
    UGameplayStatics::OpenLevel(GetWorld(), FName("CrosshairSettingsMap"));
}

void UOptionsMenuWidget::OnViewmodelClicked()
{
    UGameplayStatics::OpenLevel(GetWorld(), FName("ArmsConfigMap"));
}

void UOptionsMenuWidget::OnVideoClicked()
{
    //UE_LOG(LogTemp, Warning, TEXT("Video settings not implemented yet"));
}

void UOptionsMenuWidget::OnSoundClicked()
{
    //UE_LOG(LogTemp, Warning, TEXT("Sound settings not implemented yet"));
}

void UOptionsMenuWidget::OnInputClicked()
{
    //UE_LOG(LogTemp, Warning, TEXT("Input settings not implemented yet"));
}

void UOptionsMenuWidget::OnMultiplayerClicked()
{
    //UGameplayStatics::OpenLevel(GetWorld(), FName("MultiplayerSettings"));
}