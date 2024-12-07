#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SessionBrowserWidget.h"
#include "NetworkPlayerManager.h"  
#include "SessionGameMode.generated.h"

UCLASS()
class MASTERYOFWAR_API ASessionGameMode : public AGameModeBase
{
	GENERATED_BODY()
    
public:
	ASessionGameMode();

	virtual void BeginPlay() override;
	
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	
	void HandleNewPlayerJoined(int32 PlayerId);
	void HandlePlayerLeft(int32 PlayerId);
	void UpdatePlayerState(const FNetworkPlayerState& State);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<USessionBrowserWidget> SessionBrowserWidgetClass;

	UPROPERTY()
	ANetworkPlayerManager* PlayerManager;
	
private:
	UPROPERTY()
	USessionBrowserWidget* SessionBrowserWidget;

	void InitializeNetworking();
	void SetupNetworkCallbacks();
};