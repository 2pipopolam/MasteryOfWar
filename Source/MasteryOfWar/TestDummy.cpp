#include "TestDummy.h"
#include "DrawDebugHelpers.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "WeaponConfig.h"
#include "WeaponSystem.h"

ATestDummy::ATestDummy()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create collision components for hit zones
    HeadCollision = CreateDefaultSubobject<USphereComponent>(TEXT("HeadCollision"));
    HeadCollision->SetupAttachment(GetRootComponent());
    HeadCollision->SetSphereRadius(15.0f);
    HeadCollision->SetRelativeLocation(FVector(0.0f, 0.0f, 160.0f));

    BodyCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("BodyCollision"));
    BodyCollision->SetupAttachment(GetRootComponent());
    BodyCollision->SetCapsuleSize(25.0f, 35.0f);
    BodyCollision->SetRelativeLocation(FVector(0.0f, 0.0f, 90.0f));

    LeftArmCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("LeftArmCollision"));
    LeftArmCollision->SetupAttachment(GetRootComponent());
    LeftArmCollision->SetCapsuleSize(10.0f, 20.0f);
    LeftArmCollision->SetRelativeLocation(FVector(0.0f, -40.0f, 110.0f));

    RightArmCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("RightArmCollision"));
    RightArmCollision->SetupAttachment(GetRootComponent());
    RightArmCollision->SetCapsuleSize(10.0f, 20.0f);
    RightArmCollision->SetRelativeLocation(FVector(0.0f, 40.0f, 110.0f));

    LeftLegCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("LeftLegCollision"));
    LeftLegCollision->SetupAttachment(GetRootComponent());
    LeftLegCollision->SetCapsuleSize(12.5f, 22.5f);
    LeftLegCollision->SetRelativeLocation(FVector(0.0f, -20.0f, 45.0f));

    RightLegCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("RightLegCollision"));
    RightLegCollision->SetupAttachment(GetRootComponent());
    RightLegCollision->SetCapsuleSize(12.5f, 22.5f);
    RightLegCollision->SetRelativeLocation(FVector(0.0f, 20.0f, 45.0f));

    // Setup collision settings for all components
    TArray<UPrimitiveComponent*> AllCollisions = {
        HeadCollision, BodyCollision,
        LeftArmCollision, RightArmCollision,
        LeftLegCollision, RightLegCollision
    };

    for (auto* Collision : AllCollisions)
    {
        if (Collision)
        {
            Collision->SetCollisionProfileName(TEXT("BlockAll"));
            Collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            Collision->SetGenerateOverlapEvents(true);
        }
    }

    WeaponDamageConfig = FWeaponDamageConfig();

    // Initialize health
    CurrentHealth = MaxHealth;
}

void ATestDummy::BeginPlay()
{
    Super::BeginPlay();

    // Save debug text for later reference
    DebugText = TEXT("Test Dummy\nShoot me!");
    
    // Draw initial debug text above dummy
    DrawDebugString(
        GetWorld(),
        GetActorLocation() + FVector(0, 0, 200),
        *DebugText,
        nullptr,
        FColor::Green,
        -1.0f,
        true,
        1.0f
    );

    //UE_LOG(LogTemp, Warning, TEXT("TestDummy spawned: %s"), *GetName());
}

void ATestDummy::ClearDebugText()
{
    // Clear persistent debug text by drawing empty string
    DrawDebugString(
        GetWorld(),
        GetActorLocation() + FVector(0, 0, 200),
        TEXT(""),
        nullptr,
        FColor::Green,
        0.0f,
        true,
        1.0f
    );
}

EHitZone ATestDummy::GetHitZoneFromComponent(UPrimitiveComponent* HitComponent) const
{
    // Determine hit zone based on which collision component was hit
    if (HitComponent == HeadCollision)
        return EHitZone::Head;
    else if (HitComponent == LeftLegCollision || HitComponent == RightLegCollision)
        return EHitZone::Legs;
    else if (HitComponent == LeftArmCollision || HitComponent == RightArmCollision)
        return EHitZone::Arms;
    else if (HitComponent == BodyCollision)
        return EHitZone::Body;
        
    return EHitZone::Body; // Default to body damage if unknown component
}



float ATestDummy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    // Initialize base damage
    int32 ActualDamage = FMath::RoundToInt(DamageAmount);

    // Log initial damage and health values
    //UE_LOG(LogTemp, Warning, TEXT("===== DUMMY TAKING BASE DAMAGE: %d ====="), ActualDamage);
    //UE_LOG(LogTemp, Warning, TEXT("Current Health before damage: %.1f"), CurrentHealth);

    // Check if it's point damage
    if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
    {
        const FPointDamageEvent* PointDamageEvent = static_cast<const FPointDamageEvent*>(&DamageEvent);
        if (PointDamageEvent)
        {
            UPrimitiveComponent* HitComponent = PointDamageEvent->HitInfo.Component.Get();
            if (HitComponent && DamageCauser)
            {
                // Get weapon interface from damage causer
                if (IWeaponInterface* WeaponInterface = Cast<IWeaponInterface>(DamageCauser))
                {
                    // Get weapon-specific damage config and type
                    const FWeaponDamageConfig& DamageConfig = WeaponInterface->GetDamageConfig();
                    EWeaponType WeaponType = WeaponInterface->GetWeaponType();

                    // Log weapon type for debugging
                    //UE_LOG(LogTemp, Warning, TEXT("Weapon Type: %s"), *UEnum::GetValueAsString(WeaponType));
                    
                    // Get base damage and determine hit zone
                    int32 BaseDamage = DamageConfig.BaseDamage;
                    EHitZone HitZone = GetHitZoneFromComponent(HitComponent);
                    
                    // Calculate damage multiplier based on hit zone
                    float Multiplier = 1.0f;
                    switch (HitZone)
                    {
                        case EHitZone::Head:
                            Multiplier = DamageConfig.HeadMultiplier;
                            break;
                        case EHitZone::Body:
                            Multiplier = DamageConfig.BodyMultiplier;
                            break;
                        case EHitZone::Arms:
                            Multiplier = DamageConfig.ArmsMultiplier;
                            break;
                        case EHitZone::Legs:
                            Multiplier = DamageConfig.LegsMultiplier;
                            break;
                    }

                    // Calculate final damage
                    ActualDamage = FMath::RoundToInt(static_cast<float>(BaseDamage) * Multiplier);

                    // Log damage calculation details
                    //UE_LOG(LogTemp, Warning, TEXT("Base Damage: %d"), BaseDamage);
                    //UE_LOG(LogTemp, Warning, TEXT("Hit Zone: %s"), *UEnum::GetValueAsString(HitZone));
                    //UE_LOG(LogTemp, Warning, TEXT("Damage Multiplier: %.2f"), Multiplier);
                    //UE_LOG(LogTemp, Warning, TEXT("Final Damage: %d"), ActualDamage);

                    // Apply damage to health
                    CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);

                    // Display damage information above impact point
                    FString DamageText = FString::Printf(TEXT("%d (x%.1f)\nHP: %.0f"),
                        ActualDamage, Multiplier, CurrentHealth);
                    DrawDebugString(
                        GetWorld(),
                        PointDamageEvent->HitInfo.ImpactPoint,
                        DamageText,
                        nullptr,
                        FColor::Yellow,
                        2.0f,
                        true,
                        1.5f
                    );

                    //UE_LOG(LogTemp, Warning, TEXT("Health Remaining: %.1f"), CurrentHealth);

                    // Check if dummy is destroyed
                    if (CurrentHealth <= 0.0f)
                    {
                        //UE_LOG(LogTemp, Warning, TEXT("Dummy destroyed!"));

                        // Clear existing debug text
                        ClearDebugText();

                        // Display destruction message
                        DrawDebugString(
                            GetWorld(),
                            GetActorLocation() + FVector(0, 0, 200),
                            TEXT("DESTROYED!"),
                            nullptr,
                            FColor::Red,
                            2.0f,
                            true,
                            2.0f
                        );

                        // Visual effect for destruction
                        DrawDebugSphere(
                            GetWorld(),
                            GetActorLocation(),
                            100.0f,
                            12,
                            FColor::Red,
                            false,
                            2.0f,
                            0,
                            2.0f
                        );

                        // Destroy with delay
                        FTimerHandle TimerHandle;
                        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
                        {
                            Destroy();
                        }, 0.2f, false);
                    }
                }
            }
        }
    }

    // Update damage statistics
    TotalDamageReceived += ActualDamage;
    LastDamageReceived = ActualDamage;
    HitCount++;

    // Call parent implementation
    return Super::TakeDamage(static_cast<float>(ActualDamage), DamageEvent, EventInstigator, DamageCauser);
}