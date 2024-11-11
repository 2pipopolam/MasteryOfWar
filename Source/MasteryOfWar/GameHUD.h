#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AmmoWidget.h"
#include "Crosshair.h"
#include "GameHUD.generated.h"

UCLASS()
class MASTERYOFWAR_API AGameHUD : public AHUD
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UCrosshair> CrosshairClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UAmmoWidget> AmmoWidgetClass;

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	UCrosshair* CrosshairWidget;

	UPROPERTY()
	UAmmoWidget* AmmoWidget;

public:
	// Функции для обновления ammo из вашего оружия/персонажа
	void UpdateAmmoCount(int32 CurrentAmmo, int32 MaxAmmo);
    
	// Получить виджет прицела (если понадобится обновить его из других классов)
	UCrosshair* GetCrosshairWidget() const { return CrosshairWidget; }
};