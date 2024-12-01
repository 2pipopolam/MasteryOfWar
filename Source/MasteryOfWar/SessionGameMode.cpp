#include "SessionGameMode.h"
#include "MofWGameInstance.h"

ASessionGameMode::ASessionGameMode()
{
	PrimaryActorTick.bCanEverTick = false;
}


void ASessionGameMode::BeginPlay()
{
	Super::BeginPlay();
    
	if (SessionBrowserWidgetClass)
	{
		APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
		if (PlayerController)
		{
			SessionBrowserWidget = CreateWidget<USessionBrowserWidget>(PlayerController, SessionBrowserWidgetClass);
			if (SessionBrowserWidget)
			{
				if (UMasteryOfWarGameInstance* GameInstance = Cast<UMasteryOfWarGameInstance>(GetGameInstance()))
				{
					EGameMapType CurrentMode = GameInstance->GetCurrentGameMode();
					UE_LOG(LogTemp, Warning, TEXT("Setting up session browser for mode: %d"), static_cast<int32>(CurrentMode));
					SessionBrowserWidget->SetupForGameMode(CurrentMode);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Failed to get game instance"));
				}
                
				SessionBrowserWidget->AddToViewport();
                
				// Настраиваем ввод для UI
				FInputModeUIOnly InputMode;
				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				PlayerController->SetInputMode(InputMode);
				PlayerController->bShowMouseCursor = true;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to create session browser widget"));
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SessionBrowserWidgetClass not set in Blueprint"));
	}
}