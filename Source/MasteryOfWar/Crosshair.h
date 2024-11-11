#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "CrosshairSettingsTypes.h"
#include "Crosshair.generated.h"

UCLASS(Blueprintable)
class MASTERYOFWAR_API UCrosshair : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* CrosshairHorizontal;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* CrosshairVertical;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* CenterDot;

public:
	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	virtual void UpdateCrosshairAppearance(const FCrosshairSettings& Settings);

	virtual void NativeConstruct() override;
};