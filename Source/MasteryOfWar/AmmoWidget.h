/*
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "AmmoWidget.generated.h"

UCLASS()
class MASTERYOFWAR_API UAmmoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void UpdateAmmoCount(int32 CurrentAmmo, int32 MaxAmmo);

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* AmmoText;

private:
	FLinearColor GetAmmoColor(int32 CurrentAmmo, int32 MaxAmmo) const;
};

 */