#include "Grenade.h"
#include "GrenadeProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "NetworkClient.h"
#include "MofWGameInstance.h"
#include "MasteryOfWarCharacter.h"
#include "NetworkStructs.h"


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

    FVector SpawnLocation = CameraLocation + 
                           (ForwardVector * 30.0f) +
                           (UpVector) +
                           (RightVector);
    
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

        FVector LaunchVelocity;
        if (UProjectileMovementComponent* ProjectileMovement = Projectile->GetProjectileMovement())
        {
            FVector AimPoint = CameraLocation + ForwardVector * 1000.0f;
            FVector ThrowDirection = (AimPoint - SpawnLocation).GetSafeNormal();
            LaunchVelocity = ThrowDirection * GrenadeConfig.ThrowForce;
            
            ProjectileMovement->Velocity = LaunchVelocity;
            ProjectileMovement->bSimulationEnabled = true;
        }

        // Отправка сетевого сообщения
        if (AMasteryOfWarCharacter* MOWCharacter = Cast<AMasteryOfWarCharacter>(Character))
        {
            if (MOWCharacter->IsLocallyControlled())
            {
                if (UMasteryOfWarGameInstance* GameInstance = Cast<UMasteryOfWarGameInstance>(GetGameInstance()))
                {
                    if (NetworkClient* Client = GameInstance->GetNetworkClient())
                    {
                        FNetworkGrenadeInfo GrenadeInfo;
                        GrenadeInfo.ThrowerId = MOWCharacter->GetPlayerId();
                        GrenadeInfo.Location = SpawnLocation;
                        GrenadeInfo.Rotation = CameraRotation;
                        GrenadeInfo.Velocity = LaunchVelocity;
                        
                        Client->SendGrenadeThrow(GrenadeInfo);
                    }
                }
            }
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


void AGrenade::SimulateThrow(const FNetworkGrenadeInfo& GrenadeInfo)
{
    if (!CanFire())
    {
        return;
    }

    bCanThrow = false;
    CurrentAmmo--;

    ACharacter* Character = Cast<ACharacter>(GetOwner());
    if (!Character)
    {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = Character;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    if (AGrenadeProjectile* Projectile = GetWorld()->SpawnActor<AGrenadeProjectile>(
        GrenadeProjectileClass, GrenadeInfo.Location, FRotator(GrenadeInfo.Rotation), SpawnParams))
    {
        if (WeaponMesh)
        {
            WeaponMesh->SetVisibility(false);
        }

        Projectile->Initialize(GrenadeConfig);

        if (UProjectileMovementComponent* ProjectileMovement = Projectile->GetProjectileMovement())
        {
            ProjectileMovement->Velocity = GrenadeInfo.Velocity;
            ProjectileMovement->bSimulationEnabled = true;
        }

        if (ThrowSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, ThrowSound, GrenadeInfo.Location);
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