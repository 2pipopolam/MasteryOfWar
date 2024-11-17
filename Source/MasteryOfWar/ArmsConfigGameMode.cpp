#include "ArmsConfigGameMode.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

void AArmsConfigGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (ConfigWidgetClass)
	{
		UArmsPositionConfigWidget* ConfigWidget = CreateWidget<UArmsPositionConfigWidget>(GetWorld(), ConfigWidgetClass);
		if (ConfigWidget)
		{
			ConfigWidget->AddToViewport();

			// Set input mode to UI only
			if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
			{
				FInputModeUIOnly InputMode;
				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				PC->SetInputMode(InputMode);
				PC->SetShowMouseCursor(true);
			}
		}
	}
}