#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SessionBrowserWidget.h"
#include "SessionGameMode.generated.h"

UCLASS()
class MASTERYOFWAR_API ASessionGameMode : public AGameModeBase
{
	GENERATED_BODY()
    
public:
	ASessionGameMode();

	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<USessionBrowserWidget> SessionBrowserWidgetClass;

private:
	UPROPERTY()
	USessionBrowserWidget* SessionBrowserWidget;
};