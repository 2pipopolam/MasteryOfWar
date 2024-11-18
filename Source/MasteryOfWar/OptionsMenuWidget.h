

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OptionsMenuWidget.generated.h"

UCLASS()
class  MASTERYOFWAR_API   UOptionsMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	class UButton* BackButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* CrosshairButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* ViewmodelButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* VideoButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* SoundButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* InputButton;

	UPROPERTY(meta = (BindWidget))
	class UButton* MultiplayerButton;

	UFUNCTION()
	void OnBackClicked();

	UFUNCTION()
	void OnCrosshairClicked();

	UFUNCTION()
	void OnViewmodelClicked();

	UFUNCTION()
	void OnVideoClicked();

	UFUNCTION()
	void OnSoundClicked();

	UFUNCTION()
	void OnInputClicked();

	UFUNCTION()
	void OnMultiplayerClicked();
};
