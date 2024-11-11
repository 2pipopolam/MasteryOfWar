// Copyright Epic Games, Inc. All Rights Reserved.
#include "MasteryOfWarGameMode.h"
#include "MasteryOfWarCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "GameHUD.h"

AMasteryOfWarGameMode::AMasteryOfWarGameMode()
{
	// char class
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/MofW/Blueprints/BP_MofWCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	else
	{
		DefaultPawnClass = AMasteryOfWarCharacter::StaticClass();
	}

	// Находим класс HUD
	static ConstructorHelpers::FClassFinder<AGameHUD> HUDClassFinder(TEXT("/Game/MofW/Blueprints/BP_GameHUD"));
	if (HUDClassFinder.Succeeded())
	{
		HUDClass = HUDClassFinder.Class;
		GameHUDClass = HUDClassFinder.Class;
	}
	else
	{
		HUDClass = AGameHUD::StaticClass();
		GameHUDClass = AGameHUD::StaticClass();
	}
}

void AMasteryOfWarGameMode::BeginPlay()
{
	Super::BeginPlay();
   
	// can create some logic
	//if (AGameHUD* GameHUD = Cast<AGameHUD>(GetWorld()->GetFirstPlayerController()->GetHUD()))
	//{
		// INIT HUD
	//}
}