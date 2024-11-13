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
	// horizontal lines (left and right)
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* CrosshairHorizontalLeft;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* CrosshairHorizontalRight;

	// vertical lines(up and down)
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* CrosshairVerticalTop;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* CrosshairVerticalBottom;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* CenterDot;

	// gap size
	UPROPERTY(EditAnywhere, Category = "Crosshair")
	float GapSize = 10.0f;

public:
	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	virtual void UpdateCrosshairAppearance(const FCrosshairSettings& Settings);

	virtual void NativeConstruct() override;
};