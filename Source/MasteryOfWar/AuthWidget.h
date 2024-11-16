#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "AuthWidget.generated.h"

UCLASS()
class MASTERYOFWAR_API UAuthWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
    
	// UI Elements
	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* NicknameInput;
    
	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* PasswordInput;
    
	UPROPERTY(meta = (BindWidget))
	UButton* SignUpButton;
    
	UPROPERTY(meta = (BindWidget))
	UButton* LoginButton;
    
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ErrorText;
    
private:
	// Button click handlers
	UFUNCTION()
	void OnSignUpClicked();
    
	UFUNCTION()
	void OnLoginClicked();
    
	// Input validation
	bool ValidateInput(const FString& Nickname, const FString& Password, FString& ErrorMessage);
    
	// UI feedback
	void ShowError(const FString& Message);
	void ClearError();
    
	// Navigation
	void GoToMainMenu();
};