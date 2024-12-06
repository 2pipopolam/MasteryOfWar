#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerSpawnPoint.generated.h"

UCLASS()
class APlayerSpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	APlayerSpawnPoint();

	UPROPERTY(EditAnywhere, Category = "Spawn")
	int32 TeamId;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	bool bIsOccupied;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UTextRenderComponent* TextComponent;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	virtual void BeginPlay() override;

	void UpdateText();

#if WITH_EDITOR
	virtual void EditorApplyRotation(const FRotator& DeltaRotation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
#endif
};