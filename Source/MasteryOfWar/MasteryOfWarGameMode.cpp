// Copyright Epic Games, Inc. All Rights Reserved.

#include "MasteryOfWarGameMode.h"
#include "MasteryOfWarCharacter.h"
#include "UObject/ConstructorHelpers.h"


AMasteryOfWarGameMode::AMasteryOfWarGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/MofW/Blueprints/BP_MofWCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	else
	{
		DefaultPawnClass = AMasteryOfWarCharacter::StaticClass();
	}
}