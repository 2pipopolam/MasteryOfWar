#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "NetworkStructs.h"
#include "SessionEntryWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJoinSessionComplete, bool, Success);

UCLASS()
class MASTERYOFWAR_API USessionEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetSessionInfo(const FSessionInfo& Info);

	UPROPERTY(BlueprintAssignable, Category = "Session")
	FOnJoinSessionComplete OnJoinSessionComplete;
    
protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* SessionNameText;
    
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* PlayerCountText;
    
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* JoinButton;
    
private:
	UFUNCTION()
	void OnJoinClicked();
    
	// Добавляем объявление метода
	void HandleSessionJoined(bool Success, int32 SessionId);
    
	FSessionInfo SessionInfo;
	class UMasteryOfWarGameInstance* GameInstance;
};