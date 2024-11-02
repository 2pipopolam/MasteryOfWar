// Copyright Epic Games, Inc. All Rights Reserved.

#include "MasteryOfWarGameMode.h"
#include "MasteryOfWarCharacter.h"
#include "UObject/ConstructorHelpers.h"


AMasteryOfWarGameMode::AMasteryOfWarGameMode()
{
	// Меняем путь на ваш новый Blueprint
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_MofWCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	else
	{
		// Если Blueprint не найден, используем C++ класс
		DefaultPawnClass = AMasteryOfWarCharacter::StaticClass();
	}
}