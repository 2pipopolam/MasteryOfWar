#include "TestDummy.h"
#include "DrawDebugHelpers.h"
#include "Engine/DamageEvents.h" 

ATestDummy::ATestDummy()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;

    HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
    BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
    LeftArmMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftArmMesh"));
    RightArmMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightArmMesh"));
    LeftLegMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftLegMesh"));
    RightLegMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightLegMesh"));

    HeadMesh->SetupAttachment(RootComponent);
    BodyMesh->SetupAttachment(RootComponent);
    LeftArmMesh->SetupAttachment(RootComponent);
    RightArmMesh->SetupAttachment(RootComponent);
    LeftLegMesh->SetupAttachment(RootComponent);
    RightLegMesh->SetupAttachment(RootComponent);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
    if (SphereMeshAsset.Succeeded())
    {
        HeadMesh->SetStaticMesh(SphereMeshAsset.Object);
        HeadMesh->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f));
        HeadMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 160.0f));
    }

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMeshAsset(TEXT("/Engine/BasicShapes/Cylinder"));
    if (CylinderMeshAsset.Succeeded())
    {
        // BODY
        BodyMesh->SetStaticMesh(CylinderMeshAsset.Object);
        BodyMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.7f));
        BodyMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 90.0f));

        // HANDS
        LeftArmMesh->SetStaticMesh(CylinderMeshAsset.Object);
        RightArmMesh->SetStaticMesh(CylinderMeshAsset.Object);
        LeftArmMesh->SetRelativeScale3D(FVector(0.2f, 0.2f, 0.4f));
        RightArmMesh->SetRelativeScale3D(FVector(0.2f, 0.2f, 0.4f));
        LeftArmMesh->SetRelativeLocation(FVector(0.0f, -40.0f, 110.0f));
        RightArmMesh->SetRelativeLocation(FVector(0.0f, 40.0f, 110.0f));

        // LEGS
        LeftLegMesh->SetStaticMesh(CylinderMeshAsset.Object);
        RightLegMesh->SetStaticMesh(CylinderMeshAsset.Object);
        LeftLegMesh->SetRelativeScale3D(FVector(0.25f, 0.25f, 0.45f));
        RightLegMesh->SetRelativeScale3D(FVector(0.25f, 0.25f, 0.45f));
        LeftLegMesh->SetRelativeLocation(FVector(0.0f, -20.0f, 45.0f));
        RightLegMesh->SetRelativeLocation(FVector(0.0f, 20.0f, 45.0f));
    }

    TArray<UStaticMeshComponent*> AllMeshes = {HeadMesh, BodyMesh, LeftArmMesh, RightArmMesh, LeftLegMesh, RightLegMesh};
    for (auto* Mesh : AllMeshes)
    {
        Mesh->SetCollisionProfileName(TEXT("BlockAll"));
        Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Mesh->SetGenerateOverlapEvents(true);
    }

    UE_LOG(LogTemp, Error, TEXT("TestDummy Constructor - Collision Setup Complete"));
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

    UE_LOG(LogTemp, Error, TEXT("TestDummy BeginPlay Called"));
}

EHitZone ATestDummy::GetHitZoneFromComponent(UPrimitiveComponent* HitComponent) const
{
    if (HitComponent == HeadMesh)
        return EHitZone::Head;
    else if (HitComponent == LeftLegMesh || HitComponent == RightLegMesh)
        return EHitZone::Legs;
    else if (HitComponent == LeftArmMesh || HitComponent == RightArmMesh)
        return EHitZone::Arms;
    else
        return EHitZone::Body;
}


float ATestDummy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, 
                            AController* EventInstigator, AActor* DamageCauser)
{
    int32 ActualDamage = FMath::RoundToInt(DamageAmount); // Начальное округление

    UE_LOG(LogTemp, Error, TEXT("===== DUMMY TAKING BASE DAMAGE: %d ====="), ActualDamage);

    if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
    {
        const FPointDamageEvent* PointDamageEvent = static_cast<const FPointDamageEvent*>(&DamageEvent);
        if (PointDamageEvent && PointDamageEvent->HitInfo.Component.Get())
        {
            EHitZone HitZone = GetHitZoneFromComponent(PointDamageEvent->HitInfo.Component.Get());
            
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

            // round damage
            ActualDamage = FMath::RoundToInt(static_cast<float>(ActualDamage) * Multiplier);
            
            UE_LOG(LogTemp, Error, TEXT("Hit Zone: %s"), *HitZoneString);
            UE_LOG(LogTemp, Error, TEXT("Damage Multiplier: %f"), Multiplier);
            UE_LOG(LogTemp, Error, TEXT("Final Damage: %d"), ActualDamage);
            
            // show damage
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

    TotalDamageReceived += ActualDamage;
    LastDamageReceived = ActualDamage;
    HitCount++;

    Super::TakeDamage(static_cast<float>(ActualDamage), DamageEvent, EventInstigator, DamageCauser);
    return static_cast<float>(ActualDamage);
}