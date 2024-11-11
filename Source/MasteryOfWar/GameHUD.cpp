#include "GameHUD.h"
#include "CrosshairSaveSettings.h"
#include "Kismet/GameplayStatics.h"

void AGameHUD::BeginPlay()
{
	Super::BeginPlay();

	// Create Crosshair
	if (CrosshairClass)
	{
		CrosshairWidget = CreateWidget<UCrosshair>(GetWorld(), CrosshairClass);
		if (CrosshairWidget)
		{
			CrosshairWidget->AddToViewport();
            
			// Load saved crosshair settings
			UCrosshairSaveSettings* SaveSettings = Cast<UCrosshairSaveSettings>(
				UGameplayStatics::LoadGameFromSlot("CrosshairSettings", 0));
                
			if (SaveSettings)
			{
				CrosshairWidget->UpdateCrosshairAppearance(SaveSettings->SavedSettings);
			}
		}
	}

	// Create Ammo Widget
	if (AmmoWidgetClass)
	{
		AmmoWidget = CreateWidget<UAmmoWidget>(GetWorld(), AmmoWidgetClass);
		if (AmmoWidget)
		{
			AmmoWidget->AddToViewport();
		}
	}
}

void AGameHUD::UpdateAmmoCount(int32 CurrentAmmo, int32 MaxAmmo)
{
	if (AmmoWidget)
	{
		AmmoWidget->UpdateAmmoCount(CurrentAmmo, MaxAmmo);
	}
}