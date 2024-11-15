#include "DesertEagle.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "BulletFireBehavior.h"

ADesertEagle::ADesertEagle()
{
    // Create and set fire behavior
    UBulletFireBehavior* FireBehaviorObj = NewObject<UBulletFireBehavior>();
    if (FireBehaviorObj)
    {
        FireBehavior = FireBehaviorObj;
    }

    InitializeWeaponConfig();
    LoadWeaponAssets();
}

void ADesertEagle::InitializeWeaponConfig()
{
    DesertEagleConfig = FDesertEagleConfig(); // This will set all the default values
    WeaponConfig = DesertEagleConfig; // Initialize base weapon config
}

void ADesertEagle::LoadWeaponAssets()
{
    // Upload static mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Game/Weapons/DesertEagle/DesertEagle"));
    if (MeshAsset.Succeeded() && WeaponMesh)
    {
        WeaponMesh->SetStaticMesh(MeshAsset.Object);
    }

    // Effects
    static ConstructorHelpers::FObjectFinder<UParticleSystem> MuzzleFlashFX(TEXT("/Game/Weapons/Particles/Fire"));
    if (MuzzleFlashFX.Succeeded())
    {
        MuzzleFlashTemplate = MuzzleFlashFX.Object;
    }

    static ConstructorHelpers::FObjectFinder<UParticleSystem> ShellEjectFX(TEXT("/Game/Effects/Particles/P_ShellEject_DesertEagle"));
    if (ShellEjectFX.Succeeded())
    {
        EjectedShellEffect = ShellEjectFX.Object;
    }

    // Sounds
    static ConstructorHelpers::FObjectFinder<USoundBase> FireSFX(TEXT("/Game/Weapons/Sounds/desert_eagle_shooting"));
    if (FireSFX.Succeeded())
    {
        FireSound = FireSFX.Object;
    }

    static ConstructorHelpers::FObjectFinder<USoundBase> EmptySFX(TEXT("/Game/Weapons/Sounds/empty_mag_sound"));
    if (EmptySFX.Succeeded())
    {
        EmptyMagazineSound = EmptySFX.Object;
    }

    static ConstructorHelpers::FObjectFinder<USoundBase> ReloadSFX(TEXT("/Game/Weapons/Sounds/desert_eagle_reloading"));
    if (ReloadSFX.Succeeded())
    {
        ReloadSound = ReloadSFX.Object;
    }

    // Animations
    static ConstructorHelpers::FObjectFinder<UAnimMontage> ReloadAnim(TEXT("/Game/Animations/AM_DesertEagle_Reload"));
    if (ReloadAnim.Succeeded())
    {
        ReloadAnimation = ReloadAnim.Object;
    }

    static ConstructorHelpers::FObjectFinder<UAnimMontage> FireAnim(TEXT("/Game/Animations/AM_DesertEagle_Fire"));
    if (FireAnim.Succeeded())
    {
        FireAnimation = FireAnim.Object;
    }

    // Bullet
    static ConstructorHelpers::FClassFinder<ABullet> BulletBPClass(TEXT("/Game/Weapons/Blueprints/BP_Bullet"));
    if (BulletBPClass.Succeeded())
    {
        BulletClass = BulletBPClass.Class;
    }

    // Widget
    static ConstructorHelpers::FClassFinder<UAmmoWidget> WidgetClassFinder(TEXT("/Game/MofW/Blueprints/WBP_AmmoWidget"));
    if(WidgetClassFinder.Succeeded())
    {
        AmmoWidgetClass = WidgetClassFinder.Class;
    }
}

void ADesertEagle::BeginPlay()
{
    Super::BeginPlay();
    SetupWeaponCollision();
    
    // Camera check and init 
    if (CameraRecoilComponent)
    {
        ensure(DesertEagleConfig.RecoilPattern.PatternPoints.Num() > 0);
        
        CameraRecoilComponent->SetRecoilPattern(DesertEagleConfig.RecoilPattern);
        
        if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
        {
            if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
            {
                if (UCameraComponent* Camera = OwnerPawn->FindComponentByClass<UCameraComponent>())
                {
                    CameraRecoilComponent->SetTargetCamera(Camera);
                }
            }
        }
    }
}

void ADesertEagle::Fire()
{
    if (!CanFire())
    {
        if (EmptyMagazineSound && !MagazineState.bIsReloading)
        {
            UGameplayStatics::PlaySoundAtLocation(this, EmptyMagazineSound, GetActorLocation());
        }
        return;
    }

    Super::Fire();
}

void ADesertEagle::PlayFireEffects()
{
    Super::PlayFireEffects();

    // Additional Desert Eagle specific effects
    if (EjectedShellEffect)
    {
        FTransform ShellTransform = GetShellEjectSocketTransform();
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            EjectedShellEffect,
            ShellTransform.GetLocation(),
            ShellTransform.GetRotation().Rotator()
        );
    }
}

void ADesertEagle::StartFiring()
{
    if (!MagazineState.bIsReloading)
    {
        Super::StartFiring();
    }
}

void ADesertEagle::StopFiring()
{
    Super::StopFiring();
}

void ADesertEagle::Reload()
{
    if (MagazineState.CurrentAmmo == MagazineState.MaxAmmo || MagazineState.bIsReloading)
    {
        return;
    }

    Super::Reload();
    OnReloadComplete.Broadcast();
}

bool ADesertEagle::CanFire() const
{
    return Super::CanFire() && !MagazineState.bIsReloading;
}

void ADesertEagle::PlayReloadEffects()
{
    Super::PlayReloadEffects();
}

void ADesertEagle::SetupWeaponCollision()
{
    if (WeaponMesh)
    {
        WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    }
}