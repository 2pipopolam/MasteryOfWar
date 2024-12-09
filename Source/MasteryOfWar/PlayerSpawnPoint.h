#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/TextRenderComponent.h"
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

	// Добавляем коллизию и спрайт
	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UBoxComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UBillboardComponent* SpriteComponent;

	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void SetOccupied(bool bNewOccupied) { bIsOccupied = bNewOccupied; }
    
	UFUNCTION(BlueprintCallable, Category = "Spawn")
	bool IsOccupied() const { return bIsOccupied; }

	UFUNCTION(BlueprintCallable, Category = "Spawn")
	int32 GetTeamId() const { return TeamId; }

	// Добавляем метод для получения трансформации спавна
	UFUNCTION(BlueprintCallable, Category = "Spawn")
	FTransform GetSpawnTransform() const;

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