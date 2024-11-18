#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "OptionsMenuGameMode.generated.h"

UCLASS()
class MASTERYOFWAR_API AOptionsMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AOptionsMenuGameMode();

	virtual void BeginPlay() override;

protected:
	// widget class
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UUserWidget> OptionsMenuWidgetClass;

	// pointer to widget
	UPROPERTY()
	class UUserWidget* OptionsMenuWidget;
};
