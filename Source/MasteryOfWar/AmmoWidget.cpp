/*
#include "AmmoWidget.h"
#include "Components/TextBlock.h"

void UAmmoWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UAmmoWidget::UpdateAmmoCount(int32 CurrentAmmo, int32 MaxAmmo)
{
	if (!AmmoText) return;

	// text update
	FString AmmoString = FString::Printf(TEXT("%d/%d"), CurrentAmmo, MaxAmmo);
	AmmoText->SetText(FText::FromString(AmmoString));

	// color update
	FLinearColor NewColor = GetAmmoColor(CurrentAmmo, MaxAmmo);
	AmmoText->SetColorAndOpacity(FSlateColor(NewColor));
}

FLinearColor UAmmoWidget::GetAmmoColor(int32 CurrentAmmo, int32 MaxAmmo) const
{
	float Ratio = static_cast<float>(CurrentAmmo) / static_cast<float>(MaxAmmo);
    
	if (Ratio > 0.5f)
	{
		// from green to yellow (100% -> 50%)
		float GreenIntensity = (Ratio - 0.5f) * 2.0f;
		return FLinearColor(1.0f - GreenIntensity, 1.0f, 0.0f);
	}
	else
	{
		// from yellow to red (50% -> 0%)
		float RedIntensity = Ratio * 2.0f;
		return FLinearColor(1.0f, RedIntensity, 0.0f);
	}
}
*/