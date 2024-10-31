#include "ShootingComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UShootingComponent::UShootingComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

UShootingComponent::~UShootingComponent()
{
}

void UShootingComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UShootingComponent::Shoot()
{
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter) return;

    // Get camera
    UCameraComponent* CameraComp = OwnerCharacter->FindComponentByClass<UCameraComponent>();
    if (!CameraComp) return;

    FVector CameraLocation = CameraComp->GetComponentLocation();
    FRotator CameraRotation = CameraComp->GetComponentRotation();
    
    //ray tracing parameters 
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.bTraceComplex = true;
    QueryParams.bReturnPhysicalMaterial = true;

    FHitResult HitResult;
    FVector StartTrace = CameraLocation;
    FVector ForwardVector = CameraRotation.Vector();
    FVector EndTrace = StartTrace + (ForwardVector * WeaponRange);

    // exec ray tracing 
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartTrace,
        EndTrace,
        ECC_Visibility,
        QueryParams
    );

    // muzzle effect after shot
    if (MuzzleFlash)
    {
        UGameplayStatics::SpawnEmitterAttached(
            MuzzleFlash,
            OwnerCharacter->GetMesh(),
            FName("MuzzleSocket"),
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::SnapToTarget
        );
    }

    if (bHit)
    {
        //hit effect
        if (ImpactEffect)
        {
            UGameplayStatics::SpawnEmitterAtLocation(
                GetWorld(),
                ImpactEffect,
                HitResult.Location,
                HitResult.Normal.Rotation()
            );
        }

/*      // needed actor with HP
        AActor* HitActor = HitResult.GetActor();
        if (HitActor)
        {
            FPointDamageEvent DamageEvent(WeaponDamage, HitResult, ForwardVector, nullptr);
            HitActor->TakeDamage(WeaponDamage, DamageEvent, OwnerCharacter->GetController(), OwnerCharacter);
        }
*/
        // logging
        #if WITH_EDITOR
        DrawDebugLine(
            GetWorld(),
            StartTrace,
            HitResult.Location,
            FColor::Red,
            false,
            2.0f
        );
        DrawDebugPoint(
            GetWorld(),
            HitResult.Location,
            10.0f,
            FColor::Red,
            false,
            2.0f
        );
        #endif
    }
}

