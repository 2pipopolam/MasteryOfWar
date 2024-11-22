#include "Bullet.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/DamageType.h"  
#include "Engine/DamageEvents.h"
#include "TestDummy.h"


ABullet::ABullet()
{
    PrimaryActorTick.bCanEverTick = true;

    BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ARAmmo"));
    RootComponent = BulletMesh;
    
    // set mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
    if (MeshAsset.Succeeded())
    {
        BulletMesh->SetStaticMesh(MeshAsset.Object);
        BulletMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
    }
    
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->SetUpdatedComponent(BulletMesh);
    ProjectileMovement->InitialSpeed = 8000.0f;
    ProjectileMovement->MaxSpeed = 8000.0f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->ProjectileGravityScale = 0.0f;
    ProjectileMovement->bShouldBounce = false;

    // collisions
    BulletMesh->SetCollisionProfileName(TEXT("BlockAll"));
    BulletMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    BulletMesh->SetGenerateOverlapEvents(true);
    
    BulletMesh->SetSimulatePhysics(false);
    
    BulletMesh->OnComponentHit.AddDynamic(this, &ABullet::OnBulletHit);
    
    InitialLifeSpan = 5.0f;
}



void ABullet::BeginPlay()
{
    Super::BeginPlay();
    StartLocation = GetActorLocation();
    
    // logging
    UE_LOG(LogTemp, Warning, TEXT("Bullet spawned at location: %s with rotation: %s"), 
           *GetActorLocation().ToString(), *GetActorRotation().ToString());
    
    if (BulletMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("Bullet mesh is valid. Scale: %s"), 
               *BulletMesh->GetRelativeScale3D().ToString());
    }
}


void ABullet::InitializeBullet(float Damage, float Speed, float MaxRange)
{
    // round damage value
    WeaponDamage = FMath::RoundToInt(Damage);
    
    ProjectileMovement->InitialSpeed = Speed;
    ProjectileMovement->MaxSpeed = Speed;
    MaxTravelDistance = MaxRange;
    StartLocation = GetActorLocation();
    
    UE_LOG(LogTemp, Error, TEXT("Bullet initialized with Damage: %d, Speed: %f, MaxRange: %f"),
           WeaponDamage, Speed, MaxRange);
}



void ABullet::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FHitResult HitResult;
    FVector Start = GetActorLocation();
    FVector End = Start + GetActorForwardVector() * 100.0f;
    
    /*DrawDebugLine(
        GetWorld(),
        Start,
        End,
        FColor::Red,
        false,
        -1,
        0,
        1.0f
    );
    */

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
    {
        if (HitResult.GetActor())
        {
            UE_LOG(LogTemp, Warning, TEXT("Line trace hit: %s at distance %f"), 
                   *HitResult.GetActor()->GetName(), HitResult.Distance);

            if (HitResult.GetActor()->GetClass()->GetName().Contains(TEXT("TestDummy")))
            {
                FPointDamageEvent DamageEvent(static_cast<float>(WeaponDamage), HitResult, GetActorForwardVector(), nullptr);
                float AppliedDamage = HitResult.GetActor()->TakeDamage(WeaponDamage, DamageEvent, 
                                                                      GetInstigatorController(), this);
                
                UE_LOG(LogTemp, Error, TEXT("Hit Test Dummy! Base Damage: %d, Applied Damage: %f"), 
                       WeaponDamage, AppliedDamage);
                
                // visualisation
                /*
                    DrawDebugString(
                    GetWorld(),
                    HitResult.ImpactPoint,
                    FString::Printf(TEXT("%d"), WeaponDamage),
                    nullptr,
                    FColor::Red,
                    2.0f,
                    true,
                    1.0f
                );
                */

                Destroy();
            }
        }
    }

    float TravelDistance = FVector::Distance(StartLocation, GetActorLocation());
    if (TravelDistance > MaxTravelDistance)
    {
        Destroy();
    }
}


void ABullet::OnBulletHit(UPrimitiveComponent* HitComp, AActor* OtherActor, 
                         UPrimitiveComponent* OtherComp, FVector NormalImpulse, 
                         const FHitResult& Hit)
{
    UE_LOG(LogTemp, Error, TEXT("OnBulletHit CALLED!"));

    if (OtherActor && OtherActor != GetOwner())
    {
        UE_LOG(LogTemp, Error, TEXT("Hit Actor: %s"), *OtherActor->GetName());
        
        // display message 
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, 
            FString::Printf(TEXT("Hit: %s"), *OtherActor->GetName()));

        // create Point Damage Event
        FPointDamageEvent DamageEvent(WeaponDamage, Hit, Hit.ImpactNormal, nullptr);
        float AppliedDamage = OtherActor->TakeDamage(WeaponDamage, DamageEvent, GetInstigatorController(), this);
        
        UE_LOG(LogTemp, Error, TEXT("Applied Hit Damage: %f"), AppliedDamage);
    }

    Destroy();
}