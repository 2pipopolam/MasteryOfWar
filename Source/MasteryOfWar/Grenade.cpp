#include "Grenade.h"
#include "GrenadeProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"

AGrenade::AGrenade()
{
    GrenadeConfig = FGrenadeConfig();
    WeaponConfig = GrenadeConfig;
    bCanThrow = true;

    CurrentAmmo = GrenadeConfig.MaxAmmo; 

    InitializeWeaponConfig();
    LoadWeaponAssets();
}

void AGrenade::InitializeWeaponConfig()
{
    GrenadeConfig = FGrenadeConfig();

    static ConstructorHelpers::FObjectFinder<UParticleSystem> ExplosionFX(TEXT("/Game/StarterContent/Particles/P_Explosion"));
    if (ExplosionFX.Succeeded())
    {
        GrenadeConfig.ExplosionEffect = ExplosionFX.Object;
    }
    
    static ConstructorHelpers::FObjectFinder<USoundBase> ExplosionSoundFX(TEXT("/Game/Weapons/Sounds/explosion_Cue"));
    if (ExplosionSoundFX.Succeeded())
    {
        GrenadeConfig.ExplosionSound = ExplosionSoundFX.Object;
    }

    WeaponConfig = GrenadeConfig;
}

void AGrenade::LoadWeaponAssets()
{
    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Game/Weapons/Grenade/GRENADE"));
    if (MeshAsset.Succeeded() && WeaponMesh)
    {
        WeaponMesh->SetStaticMesh(MeshAsset.Object);
        WeaponMesh->SetRelativeScale3D(FVector(1.0f));
    }

    static ConstructorHelpers::FClassFinder<AGrenadeProjectile> ProjectileClassFinder(
        TEXT("/Game/MofW/Blueprints/BP_GrenadeProjectile"));
    if (ProjectileClassFinder.Succeeded())
    {
        GrenadeProjectileClass = ProjectileClassFinder.Class;
    }

    static ConstructorHelpers::FObjectFinder<USoundBase> ThrowSFX(TEXT("/Game/Weapons/Sounds/grenade_throw"));
    if (ThrowSFX.Succeeded())
    {
        ThrowSound = ThrowSFX.Object;
    }
}

void AGrenade::BeginPlay()
{
    Super::BeginPlay();
    SetupWeaponCollision();
    bCanThrow = true;
}


void AGrenade::Fire()
{
    if (!CanFire())
    {
        return;
    }

    bCanThrow = false;

    CurrentAmmo--;

    ACharacter* Character = Cast<ACharacter>(GetOwner());
    APlayerController* PlayerController = Character ? Cast<APlayerController>(Character->GetController()) : nullptr;
    UCameraComponent* Camera = Character ? Character->FindComponentByClass<UCameraComponent>() : nullptr;

    if (!Character || !PlayerController || !Camera)
    {
        return;
    }

    FVector CameraLocation;
    FRotator CameraRotation;
    PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

    FVector ForwardVector = CameraRotation.Vector();
    FVector RightVector = CameraRotation.RotateVector(FVector::RightVector);
    FVector UpVector = CameraRotation.RotateVector(FVector::UpVector);

    /////////////////////////////////////////////////////////////////////////////////////
    // Spawn further forward and higher to avoid collisions
    FVector SpawnLocation = CameraLocation + 
                           (ForwardVector * 30.0f) +  // Increased forward distance
                           (UpVector ) +       // Increased height
                           (RightVector);      // Right offset

    //////////////////////////////////////////////////////////////////////////////////////
    
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = Character;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    if (AGrenadeProjectile* Projectile = GetWorld()->SpawnActor<AGrenadeProjectile>(
        GrenadeProjectileClass, SpawnLocation, CameraRotation, SpawnParams))
    {
        if (WeaponMesh)
        {
            WeaponMesh->SetVisibility(false);
        }

        Projectile->Initialize(GrenadeConfig);

        if (UProjectileMovementComponent* ProjectileMovement = Projectile->GetProjectileMovement())
        {
            // Calculate throw velocity with a higher arc
            FVector AimPoint = CameraLocation;
            FVector ThrowDirection = (AimPoint - SpawnLocation).GetSafeNormal();
            FVector LaunchVelocity = ThrowDirection * GrenadeConfig.ThrowForce;
            
            ProjectileMovement->Velocity = LaunchVelocity;
            ProjectileMovement->bSimulationEnabled = true;
        }

        if (ThrowSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, ThrowSound, SpawnLocation);
        }

        GetWorld()->GetTimerManager().SetTimer(
            RespawnTimerHandle,
            this,
            &AGrenade::RespawnGrenade,
            GrenadeConfig.RespawnDelay,
            false
        );
    }
}


void AGrenade::StartFiring()
{
    if (bCanThrow)
    {
        Fire();
    }
}

void AGrenade::StopFiring()
{
    // Empty for grenade
}

bool AGrenade::CanFire() const
{
    return bCanThrow && WeaponMesh && GrenadeProjectileClass;
}

void AGrenade::RespawnGrenade()
{
    if (WeaponMesh)
    {
        WeaponMesh->SetVisibility(true);
    }
    bCanThrow = true;
    CurrentAmmo = GrenadeConfig.MaxAmmo;
}

void AGrenade::SetupWeaponCollision()
{
    if (WeaponMesh)
    {
        WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    }
}