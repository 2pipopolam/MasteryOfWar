#include "ShootingComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

UShootingComponent::UShootingComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

UShootingComponent::~UShootingComponent()
{
    // Ensure timer is cleared when component is destroyed
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(AutoFireTimerHandle);
    }
}

void UShootingComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UShootingComponent::StartFiring()
{
    if (!bIsFiring)
    {
        bIsFiring = true;
        // Execute first shot immediately
        Shoot();
        // Start timer for automatic firing
        GetWorld()->GetTimerManager().SetTimer(
            AutoFireTimerHandle,
            this,
            &UShootingComponent::Shoot,
            FireRate,
            true // Loop the timer
        );
    }
}

void UShootingComponent::StopFiring()
{
    if (bIsFiring)
    {
        bIsFiring = false;
        // Stop the timer
        GetWorld()->GetTimerManager().ClearTimer(AutoFireTimerHandle);
    }
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

/*
        // Handle damage
        AActor* HitActor = HitResult.GetActor();
        if (HitActor)
        {
            FPointDamageEvent DamageEvent(WeaponDamage, HitResult, ForwardVector, nullptr);
            HitActor->TakeDamage(WeaponDamage, DamageEvent, OwnerCharacter->GetController(), OwnerCharacter);
        }
*/
        // Debug visualization
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