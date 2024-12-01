#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameModeConfig.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "SessionEntryWidget.h"
#include "NetworkStructs.h"
#include "MofWGameInstance.h"
#include "SessionBrowserWidget.generated.h"

UCLASS()
class MASTERYOFWAR_API USessionBrowserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetupForGameMode(EGameMapType MapType);
    
protected:
	virtual void NativeConstruct() override;
    
	// UI Components
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UScrollBox* SessionsList;
    
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* CreateSessionButton;
    
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* RefreshButton;
    
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* BackButton;
    
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UEditableTextBox* PasswordInput;
    
	// Widget class for session entries
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<USessionEntryWidget> SessionEntryWidgetClass;

private:
	UFUNCTION()
	void OnCreateSessionClicked();
    
	UFUNCTION()
	void OnRefreshClicked();
    
	UFUNCTION()
	void OnBackClicked();
    
	UFUNCTION()
	void HandleSessionsList(const TArray<FSessionInfo>& Sessions);
    
	void RefreshSessionsList();
	void AddSessionEntry(const FSessionInfo& SessionInfo);
    
	EGameMapType CurrentMapType;
	class UMasteryOfWarGameInstance* GameInstance;

	void UpdateCurrentMapType()
	{
		if (GameInstance)
		{
			CurrentMapType = GameInstance->GetCurrentGameMode();
			UE_LOG(LogTemp, Warning, TEXT("Current map type updated to: %d"), static_cast<int32>(CurrentMapType));
		}
	}

	FTimerHandle RefreshTimerHandle;
};