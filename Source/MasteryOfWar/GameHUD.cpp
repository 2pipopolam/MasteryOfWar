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
			CrosshairWidget->AddToViewport(100);
            
			
			if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(CrosshairWidget->Slot))
			{
				// add to center 
				CanvasSlot->SetAnchors(FAnchors(0.5f));
				CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
				CanvasSlot->SetPosition(FVector2D(0.0f, 0.0f));
                
				// widget size
				FVector2D ViewportSize;
				GEngine->GameViewport->GetViewportSize(ViewportSize);
				CanvasSlot->SetSize(FVector2D(ViewportSize.X * 0.2f, ViewportSize.Y * 0.2f)); // Настройте размер по необходимости
			}
            
			UpdateCrosshairFromSavedSettings();
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



void AGameHUD::UpdateCrosshairFromSavedSettings()
{
	if (!CrosshairWidget)
		return;

	UCrosshairSaveSettings* SaveSettings = Cast<UCrosshairSaveSettings>(
		UGameplayStatics::LoadGameFromSlot("CrosshairSettings", 0));

	if (SaveSettings)
	{
		CrosshairWidget->UpdateCrosshairAppearance(SaveSettings->SavedSettings);
	}
	else
	{
		FCrosshairSettings DefaultSettings;
		CrosshairWidget->UpdateCrosshairAppearance(DefaultSettings);
	}
}

void AGameHUD::UpdateAmmoCount(int32 CurrentAmmo, int32 MaxAmmo)
{
	if (AmmoWidget)
	{
		AmmoWidget->UpdateAmmoCount(CurrentAmmo, MaxAmmo);
	}
}