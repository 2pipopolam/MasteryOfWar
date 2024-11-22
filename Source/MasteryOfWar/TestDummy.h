#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DamageEvents.h" 
#include "DamageConfig.h"
#include "TestDummy.generated.h"

UCLASS()
class MASTERYOFWAR_API ATestDummy : public AActor
{
	GENERATED_BODY()
    
public:    
	ATestDummy();

protected:
	virtual void BeginPlay() override;
    
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, 
							class AController* EventInstigator, AActor* DamageCauser) override;

	// components for body parts
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* HeadMesh;
    
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BodyMesh;
    
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* LeftArmMesh;
    
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RightArmMesh;
    
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* LeftLegMesh;
    
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RightLegMesh;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* RootSceneComponent;

	// to know zone of collision
	EHitZone GetHitZoneFromComponent(UPrimitiveComponent* HitComponent) const;

	// total damage
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage")
	float TotalDamageReceived = 0.0f;

	// amount of hits
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage")
	int32 HitCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage")
	float LastDamageReceived = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	FHitZoneMultipliers HitZoneMultipliers;
};