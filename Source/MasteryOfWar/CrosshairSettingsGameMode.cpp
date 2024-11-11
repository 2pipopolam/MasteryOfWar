#include "CrosshairSettingsGameMode.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"


void ACrosshairSettingsGameMode::BeginPlay()
{
	Super::BeginPlay();
    
	if (SettingsMenuClass)
	{
		UUserWidget* SettingsMenu = CreateWidget<UUserWidget>(GetWorld(), SettingsMenuClass);
		if (SettingsMenu)
		{
			SettingsMenu->AddToViewport();
            
			APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
			if (PlayerController)
			{
				PlayerController->SetShowMouseCursor(true);
				FInputModeUIOnly InputMode;
				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				PlayerController->SetInputMode(InputMode);
			}
		}
	}
}