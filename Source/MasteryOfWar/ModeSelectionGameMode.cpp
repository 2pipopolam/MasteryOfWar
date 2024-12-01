#include "ModeSelectionGameMode.h"
#include "MofWGameInstance.h"

AModeSelectionGameMode::AModeSelectionGameMode()
{
	PrimaryActorTick.bCanEverTick = false;
}


void AModeSelectionGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UMasteryOfWarGameInstance* GameInstance = Cast<UMasteryOfWarGameInstance>(GetGameInstance()))
	{
		if (!GameInstance->IsConnectedToServer())
		{
			if (GameInstance->InitializeNetworking(ServerIP, ServerPort))
			{
				UE_LOG(LogTemp, Log, TEXT("Successfully connected to server at %s:%d"), *ServerIP, ServerPort);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to connect to server at %s:%d"), *ServerIP, ServerPort);
			}
		}
	}
}
