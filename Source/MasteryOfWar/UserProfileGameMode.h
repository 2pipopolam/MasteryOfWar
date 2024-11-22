#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "UserProfileWidget.h"
#include "UserProfileGameMode.generated.h"

UCLASS()
class MASTERYOFWAR_API AUserProfileGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AUserProfileGameMode();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserProfileWidget> UserProfileWidgetClass;
};