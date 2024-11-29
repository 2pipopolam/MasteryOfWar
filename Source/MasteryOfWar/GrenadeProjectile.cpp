#include "GrenadeProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "DamageInterface.h"
#include "Engine/DamageEvents.h"
#include "PhysicalMaterials/PhysicalMaterial.h"


AGrenadeProjectile::AGrenadeProjectile()
{
    PrimaryActorTick.bCanEverTick = true;
    bHasExploded = false;

    GrenadeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrenadeMesh"));
    RootComponent = GrenadeMesh;

    if (GrenadeMesh)
    {
        GrenadeMesh->SetVisibility(true);
        GrenadeMesh->SetHiddenInGame(false);
        
        // Configure physics settings
        GrenadeMesh->SetSimulatePhysics(true);
        GrenadeMesh->SetEnableGravity(true);
        GrenadeMesh->SetNotifyRigidBodyCollision(true);
        GrenadeMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
        GrenadeMesh->SetMassScale(NAME_None, 1.0f);
        
        // Set initial collision responses
        GrenadeMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
        GrenadeMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
    }

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    if (ProjectileMovement)
    {
        ProjectileMovement->UpdatedComponent = GrenadeMesh;
        ProjectileMovement->InitialSpeed = 1500.0f;
        ProjectileMovement->MaxSpeed = 1500.0f;
        ProjectileMovement->bRotationFollowsVelocity = false;
        ProjectileMovement->bShouldBounce = true;
        ProjectileMovement->Bounciness = 25.0f;
        ProjectileMovement->Friction = 0.5f;
        ProjectileMovement->ProjectileGravityScale = 1.0f;
        ProjectileMovement->bSimulationEnabled = true;
        ProjectileMovement->bInitialVelocityInLocalSpace = false;
        ProjectileMovement->bComponentShouldUpdatePhysicsVolume = true;
    }
}


void AGrenadeProjectile::Initialize(const FGrenadeConfig& Config)
{
    GrenadeConfig = Config;
    
    // Verify the config was properly set
    if (!GrenadeConfig.ExplosionEffect || !GrenadeConfig.ExplosionSound)
    {
        UE_LOG(LogTemp, Warning, TEXT("Grenade initialized with missing effects"));
    }
}



void AGrenadeProjectile::BeginPlay()
{
    Super::BeginPlay();
    
    if (GrenadeMesh)
    {
        GrenadeMesh->OnComponentHit.AddDynamic(this, &AGrenadeProjectile::OnGrenadeHit);
    }
    
    SetupInitialCollision();
    
    GetWorld()->GetTimerManager().SetTimer(
        DetonationTimer,
        this,
        &AGrenadeProjectile::Explode,
        GrenadeConfig.DetonationDelay,
        false
    );
}

void AGrenadeProjectile::OnGrenadeHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, 
                                     UPrimitiveComponent* OtherComp, FVector NormalImpulse, 
                                     const FHitResult& Hit)
{
    if (GrenadeConfig.BounceSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            GrenadeConfig.BounceSound,
            GetActorLocation()
        );
    }
}


void AGrenadeProjectile::SetupInitialCollision()
{
    FTimerHandle CollisionTimer;
    GetWorld()->GetTimerManager().SetTimer(CollisionTimer, [this]()
    {
        if (GrenadeMesh && !bHasExploded)
        {
            GrenadeMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
        }
    }, 0.2f, false);
}



void AGrenadeProjectile::Explode()
{
    if (bHasExploded)
        return;

    bHasExploded = true;
    
    PerformExplosionDamage();
    SpawnExplosionEffects();
    
    Destroy();
}

void AGrenadeProjectile::PerformExplosionDamage()
{
    FVector ExplosionCenter = GetActorLocation();
    TMap<AActor*, TArray<FHitResult>> ActorHits;
    
    for (int32 i = 0; i < GrenadeConfig.NumRaycastsPerExplosion; ++i)
    {
        FVector RayDirection = FMath::VRand();
        FVector EndPoint = ExplosionCenter + (RayDirection * GrenadeConfig.ExplosionRadius);
        
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, ExplosionCenter, EndPoint, ECC_WorldStatic, QueryParams))
        {
            if (AActor* HitActor = HitResult.GetActor())
            {
                ActorHits.FindOrAdd(HitActor).Add(HitResult);
            }
            /*
            #if WITH_EDITOR
                DrawDebugLine(GetWorld(), ExplosionCenter, HitResult.Location, 
                            FColor::Red, false, 2.0f, 0, 1.0f);
            #endif
            */
        }
    }

    for (auto& Pair : ActorHits)
    {
        AActor* HitActor = Pair.Key;
        const TArray<FHitResult>& Hits = Pair.Value;
        
        float ClosestDistance = MAX_FLT;
        FHitResult* BestHit = nullptr;
        
        for (const FHitResult& Hit : Hits)
        {
            float Distance = (Hit.Location - ExplosionCenter).Size();
            if (Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
                BestHit = const_cast<FHitResult*>(&Hit);
            }
        }
        
        if (BestHit)
        {
            float BaseDamage = CalculateDamageForDistance(ClosestDistance);
            float ZoneMultiplier = 1.0f;

            if (IDamageInterface* DamageInterface = Cast<IDamageInterface>(HitActor))
            {
                if (UPrimitiveComponent* HitComponent = BestHit->Component.Get())
                {
                    EHitZone HitZone = DamageInterface->GetHitZone(HitComponent);
                    
                    switch (HitZone)
                    {
                        case EHitZone::Head:
                            ZoneMultiplier = GrenadeConfig.DamageConfig.HeadMultiplier;
                            break;
                        case EHitZone::Body:
                            ZoneMultiplier = GrenadeConfig.DamageConfig.BodyMultiplier;
                            break;
                        case EHitZone::Arms:
                            ZoneMultiplier = GrenadeConfig.DamageConfig.ArmsMultiplier;
                            break;
                        case EHitZone::Legs:
                            ZoneMultiplier = GrenadeConfig.DamageConfig.LegsMultiplier;
                            break;
                    }
                }
            }

            float FinalDamage = BaseDamage * ZoneMultiplier;
            FVector DamageDirection = (ExplosionCenter - BestHit->Location).GetSafeNormal();
            
            FPointDamageEvent PointDamageEvent;
            PointDamageEvent.DamageTypeClass = UDamageType::StaticClass();
            PointDamageEvent.HitInfo = *BestHit;
            PointDamageEvent.ShotDirection = DamageDirection;
            PointDamageEvent.Damage = FinalDamage;
            
            HitActor->TakeDamage(FinalDamage, PointDamageEvent, GetInstigatorController(), this);
            
            FString DamageText = FString::Printf(
                TEXT("%.0f (x%.1f)\nDist: %.0f"),
                FinalDamage, ZoneMultiplier, ClosestDistance
            );

            /*
            DrawDebugString(
                GetWorld(),
                BestHit->Location,
                DamageText,
                nullptr,
                FColor::Yellow,
                2.0f,
                true,
                1.5f
            );
            */
        }
    }
}

float AGrenadeProjectile::CalculateDamageForDistance(float Distance) const
{
    if (Distance <= GrenadeConfig.DamageRadiusInner)
    {
        return GrenadeConfig.DamageConfig.BaseDamage;
    }
    else if (Distance <= GrenadeConfig.DamageRadiusOuter)
    {
        float DamageFalloff = FMath::GetMappedRangeValueClamped(
            FVector2D(GrenadeConfig.DamageRadiusInner, GrenadeConfig.DamageRadiusOuter),
            FVector2D(1.0f, 0.0f),
            Distance
        );
        
        return GrenadeConfig.DamageConfig.BaseDamage * FMath::Pow(DamageFalloff, GrenadeConfig.DamageFalloffExponent);
    }
    
    return 0.0f;
}


void AGrenadeProjectile::SpawnExplosionEffects()
{
    if (GrenadeConfig.ExplosionEffect)
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            GrenadeConfig.ExplosionEffect,
            GetActorLocation(),
            FRotator::ZeroRotator,
            FVector(2.0f),  // Здесь изменяем масштаб (было FVector(1.0f))
            true
        );
    }
    
    if (GrenadeConfig.ExplosionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            GrenadeConfig.ExplosionSound,
            GetActorLocation()
        );
    }
}


const FGrenadeConfig& AGrenadeProjectile::GetGrenadeConfig() const
{
    return GrenadeConfig;
}