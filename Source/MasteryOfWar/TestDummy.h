#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/DamageEvents.h" 
#include "DamageConfig.h"
#include "WeaponConfig.h"
#include "TestDummy.generated.h"

UCLASS()
class MASTERYOFWAR_API ATestDummy : public ACharacter
{
	GENERATED_BODY()
    
public:    
	ATestDummy();

protected:
	virtual void BeginPlay() override;
    
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, 
							class AController* EventInstigator, AActor* DamageCauser) override;

	// collision components for different parts of body
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit Zones")
	class USphereComponent* HeadCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit Zones")
	class UCapsuleComponent* BodyCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit Zones")
	class UCapsuleComponent* LeftArmCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit Zones")
	class UCapsuleComponent* RightArmCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit Zones")
	class UCapsuleComponent* LeftLegCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit Zones")
	class UCapsuleComponent* RightLegCollision;

	EHitZone GetHitZoneFromComponent(UPrimitiveComponent* HitComponent) const;

	// DAMAGE
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage")
	float TotalDamageReceived = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage")
	int32 HitCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage")
	float LastDamageReceived = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	FHitZoneMultipliers HitZoneMultipliers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	float CurrentHealth = 100.0f;

	UPROPERTY()
	FString DebugText;  

	void ClearDebugText();

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	FWeaponDamageConfig WeaponDamageConfig;
};