#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GlobalArmsConfig.generated.h"

USTRUCT(BlueprintType)
struct FGlobalArmsPosition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arms Configuration")
	FVector Position = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arms Configuration")
	FRotator Rotation = FRotator::ZeroRotator;
};

UCLASS()
class MASTERYOFWAR_API UGlobalArmsConfig : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Arms Configuration")
	FGlobalArmsPosition ArmsPosition;
};