#include "SessionGameMode.h"
#include "MofWGameInstance.h"
#include "MasteryOfWarCharacter.h"

ASessionGameMode::ASessionGameMode()
{
	PrimaryActorTick.bCanEverTick = false;
}


void ASessionGameMode::BeginPlay()
{
	Super::BeginPlay();

	InitializeNetworking(); 


	if (UMasteryOfWarGameInstance* GameInstance = Cast<UMasteryOfWarGameInstance>(GetGameInstance()))
	{
		GameInstance->SetPlayerManager(PlayerManager);
	}

	
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


void ASessionGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (UMasteryOfWarGameInstance* GameInstance = Cast<UMasteryOfWarGameInstance>(GetGameInstance()))
	{
		if (NetworkClient* Client = GameInstance->GetNetworkClient())
		{
			// Request current session state
			Client->RequestSessionState();
		}
	}
}

void ASessionGameMode::InitializeNetworking()
{
	// Spawn NetworkPlayerManager
	FActorSpawnParameters SpawnParams;
	PlayerManager = GetWorld()->SpawnActor<ANetworkPlayerManager>(SpawnParams);
    
	if (UMasteryOfWarGameInstance* GameInstance = Cast<UMasteryOfWarGameInstance>(GetGameInstance()))
	{
		PlayerManager->Initialize(GameInstance);
        
		if (NetworkClient* Client = GameInstance->GetNetworkClient())
		{
			SetupNetworkCallbacks();
		}
	}
}

void ASessionGameMode::SetupNetworkCallbacks()
{
	if (UMasteryOfWarGameInstance* GameInstance = Cast<UMasteryOfWarGameInstance>(GetGameInstance()))
	{
		if (NetworkClient* Client = GameInstance->GetNetworkClient())
		{
			Client->OnPlayerJoined.AddUObject(this, &ASessionGameMode::HandleNewPlayerJoined);
			Client->OnPlayerLeft.AddUObject(this, &ASessionGameMode::HandlePlayerLeft);
			Client->OnPlayerStateReceived.AddUObject(this, &ASessionGameMode::UpdatePlayerState);
		}
	}
}

void ASessionGameMode::HandleNewPlayerJoined(int32 PlayerId)
{
	if (PlayerManager)
	{
		PlayerManager->HandlePlayerJoined(PlayerId, 0);
	}
}

void ASessionGameMode::HandlePlayerLeft(int32 PlayerId)
{
	if (PlayerManager)
	{
		PlayerManager->HandlePlayerLeft(PlayerId);
	}
}

void ASessionGameMode::UpdatePlayerState(const FNetworkPlayerState& State)
{
	if (PlayerManager)
	{
		PlayerManager->UpdatePlayerState(State);
	}
}

void ASessionGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (AMasteryOfWarCharacter* Character = Cast<AMasteryOfWarCharacter>(Exiting->GetPawn()))
	{
		PlayerManager->HandlePlayerLeft(Character->GetPlayerId());
	}
}