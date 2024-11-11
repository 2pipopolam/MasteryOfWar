#include "Crosshair.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetTree.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"

void UCrosshair::UpdateCrosshairAppearance(const FCrosshairSettings& Settings)
{
	if (CrosshairHorizontal)
	{
		CrosshairHorizontal->SetColorAndOpacity(Settings.CrosshairColor);
		FVector2D HorizontalScale(Settings.CrosshairSize, Settings.LineWidth);
		CrosshairHorizontal->SetRenderScale(HorizontalScale);
		CrosshairHorizontal->SetOpacity(Settings.Opacity);
	}

	if (CrosshairVertical)
	{
		CrosshairVertical->SetColorAndOpacity(Settings.CrosshairColor);
		FVector2D VerticalScale(Settings.LineWidth, Settings.CrosshairSize);
		CrosshairVertical->SetRenderScale(VerticalScale);
		CrosshairVertical->SetOpacity(Settings.Opacity);
	}

	if (CenterDot)
	{
		CenterDot->SetVisibility(Settings.bShowCenterDot ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
		CenterDot->SetColorAndOpacity(Settings.CrosshairColor);
		FVector2D DotScale(Settings.CenterDotSize, Settings.CenterDotSize);
		CenterDot->SetRenderScale(DotScale);
		CenterDot->SetOpacity(Settings.Opacity);
	}
}

void UCrosshair::NativeConstruct()
{
	Super::NativeConstruct();
}