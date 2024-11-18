#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "ImageUtils.h"
#include "UserProfileWidget.generated.h"

UCLASS()
class MASTERYOFWAR_API UUserProfileWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* NicknameInput;

	UPROPERTY(meta = (BindWidget))
	UImage* AvatarImage;

	UPROPERTY(meta = (BindWidget))
	UButton* ChangeAvatarButton;

	UPROPERTY(meta = (BindWidget))
	UButton* SaveNicknameButton;

private:
	UFUNCTION()
	void OnChangeAvatarClicked();

	UFUNCTION()
	void OnSaveNicknameClicked();

	void UpdateAvatarImage(const FString& ImagePath);

	UTexture2D* DefaultAvatar;
	FString CurrentAvatarPath;
};