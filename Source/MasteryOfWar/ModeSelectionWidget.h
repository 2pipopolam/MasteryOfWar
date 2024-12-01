#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "NetworkStructs.h"
#include "ModeSelectionWidget.generated.h"

UCLASS(Abstract)
class MASTERYOFWAR_API UModeSelectionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* Button_Pistol;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* Button_Rifle;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* Button_Grenade;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* Button_Back;

	
	//UPROPERTY(EditDefaultsOnly, Category = "UI")
	//TSubclassOf<class USessionBrowserWidget> SessionBrowserWidgetClass;

	UFUNCTION(BlueprintCallable, Category = "Mode Selection")
	void OnPistolModeSelected();

	UFUNCTION(BlueprintCallable, Category = "Mode Selection")
	void OnRifleModeSelected();

	UFUNCTION(BlueprintCallable, Category = "Mode Selection")
	void OnGrenadeModeSelected();

	UFUNCTION(BlueprintCallable, Category = "Mode Selection")
	void OnBackSelected();

private:
	void SetupButtonCallbacks();
	//void ShowSessionBrowser(EGameMapType MapType);
    
	//UPROPERTY()
	//class USessionBrowserWidget* CurrentSessionBrowser;
    
	UPROPERTY()
	class UMasteryOfWarGameInstance* GameInstance;
};