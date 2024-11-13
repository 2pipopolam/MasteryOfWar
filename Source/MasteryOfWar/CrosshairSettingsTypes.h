#pragma once

#include "CoreMinimal.h"
#include "CrosshairSettingsTypes.generated.h"

USTRUCT(BlueprintType)
struct FCrosshairSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair")
	FLinearColor CrosshairColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair")
	float CrosshairSize = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair")
	float LineWidth = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair")
	float Opacity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair")
	bool bShowCenterDot = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair")
	float CenterDotSize = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair")
	float GapSize = 10.0f;
};