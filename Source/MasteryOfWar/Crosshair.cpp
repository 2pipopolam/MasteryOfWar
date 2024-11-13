#include "Crosshair.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetTree.h"
#include "Materials/MaterialInstanceDynamic.h"

void UCrosshair::NativeConstruct()
{
    Super::NativeConstruct();

    //use def gap size
    const float DefaultGapSize = 10.0f;

    if (CrosshairHorizontalLeft)
    {
        if (UCanvasPanelSlot* SlotLeft = Cast<UCanvasPanelSlot>(CrosshairHorizontalLeft->Slot))
        {
            SlotLeft->SetAnchors(FAnchors(0.5f));
            SlotLeft->SetAlignment(FVector2D(0.5f, 0.5f));
            SlotLeft->SetPosition(FVector2D(-DefaultGapSize, 0.0f));
        }
    }

    if (CrosshairHorizontalRight)
    {
        if (UCanvasPanelSlot* SlotRight = Cast<UCanvasPanelSlot>(CrosshairHorizontalRight->Slot))
        {
            SlotRight->SetAnchors(FAnchors(0.5f));
            SlotRight->SetAlignment(FVector2D(0.5f, 0.5f));
            SlotRight->SetPosition(FVector2D(DefaultGapSize, 0.0f));
        }
    }

    if (CrosshairVerticalTop)
    {
        if (UCanvasPanelSlot* SlotTop = Cast<UCanvasPanelSlot>(CrosshairVerticalTop->Slot))
        {
            SlotTop->SetAnchors(FAnchors(0.5f));
            SlotTop->SetAlignment(FVector2D(0.5f, 0.5f));
            SlotTop->SetPosition(FVector2D(0.0f, -DefaultGapSize));
        }
    }

    if (CrosshairVerticalBottom)
    {
        if (UCanvasPanelSlot* SlotBottom = Cast<UCanvasPanelSlot>(CrosshairVerticalBottom->Slot))
        {
            SlotBottom->SetAnchors(FAnchors(0.5f));
            SlotBottom->SetAlignment(FVector2D(0.5f, 0.5f));
            SlotBottom->SetPosition(FVector2D(0.0f, DefaultGapSize));
        }
    }

    if (CenterDot)
    {
        if (UCanvasPanelSlot* SlotDot = Cast<UCanvasPanelSlot>(CenterDot->Slot))
        {
            SlotDot->SetAnchors(FAnchors(0.5f));
            SlotDot->SetAlignment(FVector2D(0.5f, 0.5f));
            SlotDot->SetPosition(FVector2D(0.0f, 0.0f));
        }
    }
}



void UCrosshair::UpdateCrosshairAppearance(const FCrosshairSettings& Settings)
{
    // horizontal and vertical lines use the same logic for resizing,
    // but vertical lines are rotated 90 degrees
    
    if (CrosshairHorizontalLeft)
    {
        CrosshairHorizontalLeft->SetColorAndOpacity(Settings.CrosshairColor);
        CrosshairHorizontalLeft->SetRenderScale(FVector2D(Settings.CrosshairSize, Settings.LineWidth));
        CrosshairHorizontalLeft->SetOpacity(Settings.Opacity);
        CrosshairHorizontalLeft->SetRenderTransformAngle(0.0f);
        
        if (UCanvasPanelSlot* SlotLeft = Cast<UCanvasPanelSlot>(CrosshairHorizontalLeft->Slot))
        {
            SlotLeft->SetPosition(FVector2D(-Settings.GapSize, 0.0f));
        }
    }

    if (CrosshairHorizontalRight)
    {
        CrosshairHorizontalRight->SetColorAndOpacity(Settings.CrosshairColor);
        CrosshairHorizontalRight->SetRenderScale(FVector2D(Settings.CrosshairSize, Settings.LineWidth));
        CrosshairHorizontalRight->SetOpacity(Settings.Opacity);
        CrosshairHorizontalRight->SetRenderTransformAngle(0.0f);
        
        if (UCanvasPanelSlot* SlotRight = Cast<UCanvasPanelSlot>(CrosshairHorizontalRight->Slot))
        {
            SlotRight->SetPosition(FVector2D(Settings.GapSize, 0.0f));
        }
    }

    if (CrosshairVerticalTop)
    {
        CrosshairVerticalTop->SetColorAndOpacity(Settings.CrosshairColor);
        CrosshairVerticalTop->SetRenderScale(FVector2D(Settings.CrosshairSize, Settings.LineWidth));
        CrosshairVerticalTop->SetOpacity(Settings.Opacity);
        CrosshairVerticalTop->SetRenderTransformAngle(90.0f);
        
        if (UCanvasPanelSlot* SlotTop = Cast<UCanvasPanelSlot>(CrosshairVerticalTop->Slot))
        {
            SlotTop->SetPosition(FVector2D(0.0f, -Settings.GapSize));
        }
    }

    if (CrosshairVerticalBottom)
    {
        CrosshairVerticalBottom->SetColorAndOpacity(Settings.CrosshairColor);
        CrosshairVerticalBottom->SetRenderScale(FVector2D(Settings.CrosshairSize, Settings.LineWidth));
        CrosshairVerticalBottom->SetOpacity(Settings.Opacity);
        CrosshairVerticalBottom->SetRenderTransformAngle(90.0f);
        
        if (UCanvasPanelSlot* SlotBottom = Cast<UCanvasPanelSlot>(CrosshairVerticalBottom->Slot))
        {
            SlotBottom->SetPosition(FVector2D(0.0f, Settings.GapSize));
        }
    }

    if (CenterDot)
    {
        CenterDot->SetVisibility(Settings.bShowCenterDot ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
        CenterDot->SetColorAndOpacity(Settings.CrosshairColor);
        CenterDot->SetRenderScale(FVector2D(Settings.CenterDotSize, Settings.CenterDotSize));
        CenterDot->SetOpacity(Settings.Opacity);
    }
}