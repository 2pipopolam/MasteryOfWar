// TestDummy.cpp
#include "TestDummy.h"
#include "DrawDebugHelpers.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"

ATestDummy::ATestDummy()
{
    PrimaryActorTick.bCanEverTick = false;

    // collisions 
    HeadCollision = CreateDefaultSubobject<USphereComponent>(TEXT("HeadCollision"));
    HeadCollision->SetupAttachment(GetRootComponent());
    HeadCollision->SetSphereRadius(15.0f); // radius of head , probably changed in editor
    HeadCollision->SetRelativeLocation(FVector(0.0f, 0.0f, 160.0f)); // position of head , probably changed in editor

    BodyCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("BodyCollision"));
    BodyCollision->SetupAttachment(GetRootComponent());
    BodyCollision->SetCapsuleSize(25.0f, 35.0f); // body size
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

    // add collisions to components
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
}

void ATestDummy::BeginPlay()
{
    Super::BeginPlay();

    // draw message above dummy
    DrawDebugString(
        GetWorld(),
        GetActorLocation() + FVector(0, 0, 200),
        TEXT("Test Dummy\nShoot me!"),
        nullptr,
        FColor::Green,
        -1.0f,
        true,
        1.0f
    );

    UE_LOG(LogTemp, Error, TEXT("TestDummy spawned: %s"), *GetName());
}

EHitZone ATestDummy::GetHitZoneFromComponent(UPrimitiveComponent* HitComponent) const
{
    if (HitComponent == HeadCollision)
        return EHitZone::Head;
    else if (HitComponent == LeftLegCollision || HitComponent == RightLegCollision)
        return EHitZone::Legs;
    else if (HitComponent == LeftArmCollision || HitComponent == RightArmCollision)
        return EHitZone::Arms;
    else if (HitComponent == BodyCollision)
        return EHitZone::Body;
        
    return EHitZone::Body; // by default
}

float ATestDummy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, 
                            AController* EventInstigator, AActor* DamageCauser)
{
    int32 ActualDamage = FMath::RoundToInt(DamageAmount);

    UE_LOG(LogTemp, Error, TEXT("===== DUMMY TAKING BASE DAMAGE: %d ====="), ActualDamage);

    if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
    {
        const FPointDamageEvent* PointDamageEvent = static_cast<const FPointDamageEvent*>(&DamageEvent);
        if (PointDamageEvent)
        {
            UPrimitiveComponent* HitComponent = PointDamageEvent->HitInfo.Component.Get();
            if (HitComponent)
            {
                EHitZone HitZone = GetHitZoneFromComponent(HitComponent);
                
                float Multiplier = 1.0f;
                FString HitZoneString;
                
                switch (HitZone)
                {
                    case EHitZone::Head: 
                        Multiplier = HitZoneMultipliers.HeadMultiplier;
                        HitZoneString = TEXT("Head");
                        break;
                    case EHitZone::Body: 
                        Multiplier = HitZoneMultipliers.BodyMultiplier;
                        HitZoneString = TEXT("Body");
                        break;
                    case EHitZone::Arms: 
                        Multiplier = HitZoneMultipliers.ArmsMultiplier;
                        HitZoneString = TEXT("Arms");
                        break;
                    case EHitZone::Legs: 
                        Multiplier = HitZoneMultipliers.LegsMultiplier;
                        HitZoneString = TEXT("Legs");
                        break;
                }

                ActualDamage = FMath::RoundToInt(static_cast<float>(ActualDamage) * Multiplier);
                
                UE_LOG(LogTemp, Error, TEXT("Hit Zone: %s"), *HitZoneString);
                UE_LOG(LogTemp, Error, TEXT("Damage Multiplier: %f"), Multiplier);
                UE_LOG(LogTemp, Error, TEXT("Final Damage: %d"), ActualDamage);
                
                // SHOW DAMAGE
                FString DamageText = FString::Printf(TEXT("%d (x%.1f)"), ActualDamage, Multiplier);
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
            }
        }
    }

    TotalDamageReceived += ActualDamage;
    LastDamageReceived = ActualDamage;
    HitCount++;

    Super::TakeDamage(static_cast<float>(ActualDamage), DamageEvent, EventInstigator, DamageCauser);
    return static_cast<float>(ActualDamage);
}