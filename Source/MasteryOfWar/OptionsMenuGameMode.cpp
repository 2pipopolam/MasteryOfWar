#include "OptionsMenuGameMode.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

AOptionsMenuGameMode::AOptionsMenuGameMode()
{
	// default settings
	PrimaryActorTick.bCanEverTick = false;
}

void AOptionsMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (OptionsMenuWidgetClass)
	{
		OptionsMenuWidget = CreateWidget<UUserWidget>(GetWorld(), OptionsMenuWidgetClass);
		if (OptionsMenuWidget)
		{
			// add widget to screen
			OptionsMenuWidget->AddToViewport();
			
			APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
			if (PlayerController)
			{
				// show cursor
				PlayerController->bShowMouseCursor = true;

				// set input mode
				FInputModeUIOnly InputMode;
				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				PlayerController->SetInputMode(InputMode);
			}
		}
	}
}