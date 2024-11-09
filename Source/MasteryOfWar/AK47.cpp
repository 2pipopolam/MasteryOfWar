#include "AK47.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "BulletFireBehavior.h"

AAK47::AAK47()
{
    // Create and set fire behavior
    UBulletFireBehavior* FireBehaviorObj = NewObject<UBulletFireBehavior>();
    if (FireBehaviorObj)
    {
        FireBehavior = FireBehaviorObj;
    }

    // Initialize AK47-specific configuration
    InitializeWeaponConfig();
    LoadWeaponAssets();
}

void AAK47::InitializeWeaponConfig()
{
    AK47Config = FAK47Config(); // This will set all the default values
    WeaponConfig = AK47Config; // Initialize base weapon config
}

void AAK47::LoadWeaponAssets()
{
    // Загружаем статический меш для оружия
    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Game/Weapons/Meshes/AK47/ak-47"));
    if (MeshAsset.Succeeded() && WeaponMesh)
    {
        WeaponMesh->SetStaticMesh(MeshAsset.Object);
    }

    //effects
    static ConstructorHelpers::FObjectFinder<UParticleSystem> MuzzleFlashFX(TEXT("/Game/Effects/Particles/P_MuzzleFlash_AK47"));
    if (MuzzleFlashFX.Succeeded())
    {
        MuzzleFlashTemplate = MuzzleFlashFX.Object;
    }

    static ConstructorHelpers::FObjectFinder<UParticleSystem> ShellEjectFX(TEXT("/Game/Effects/Particles/P_ShellEject_AK47"));
    if (ShellEjectFX.Succeeded())
    {
        EjectedShellEffect = ShellEjectFX.Object;
    }

    // sounds
    static ConstructorHelpers::FObjectFinder<USoundBase> FireSFX(TEXT("/Game/Sounds/Weapons/S_AK47_Fire"));
    if (FireSFX.Succeeded())
    {
        FireSound = FireSFX.Object;
    }

    static ConstructorHelpers::FObjectFinder<USoundBase> EmptySFX(TEXT("/Game/Weapons/Sounds/empty_mag_sound"));
    if (EmptySFX.Succeeded())
    {
        EmptyMagazineSound = EmptySFX.Object;
    }

    static ConstructorHelpers::FObjectFinder<USoundBase> ReloadSFX(TEXT("/Game/Sounds/Weapons/S_AK47_Reload"));
    if (ReloadSFX.Succeeded())
    {
        ReloadSound = ReloadSFX.Object;
    }

    // animations
    static ConstructorHelpers::FObjectFinder<UAnimMontage> ReloadAnim(TEXT("/Game/Animations/AM_AK47_Reload"));
    if (ReloadAnim.Succeeded())
    {
        ReloadAnimation = ReloadAnim.Object;
    }

    static ConstructorHelpers::FObjectFinder<UAnimMontage> FireAnim(TEXT("/Game/Animations/AM_AK47_Fire"));
    if (FireAnim.Succeeded())
    {
        FireAnimation = FireAnim.Object;
    }

    // bullet
    static ConstructorHelpers::FClassFinder<ABullet> BulletBPClass(TEXT("/Game/Weapons/Blueprints/BP_Bullet"));
    if (BulletBPClass.Succeeded())
    {
        BulletClass = BulletBPClass.Class;
    }

    // widget
    static ConstructorHelpers::FClassFinder<UAmmoWidget> WidgetClassFinder(TEXT("/Game/UI/WBP_AmmoWidget"));
    if(WidgetClassFinder.Succeeded())
    {
        AmmoWidgetClass = WidgetClassFinder.Class;
    }
}

void AAK47::BeginPlay()
{
    Super::BeginPlay();
    SetupWeaponCollision();

    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
        {
            CreateAmmoWidget(PC);
        }
    }
}

void AAK47::Fire()
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

void AAK47::PlayFireEffects()
{
    Super::PlayFireEffects();

    // Additional AK47-specific effects
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

void AAK47::StartFiring()
{
    if (!MagazineState.bIsReloading)
    {
        Super::StartFiring();
    }
}

void AAK47::StopFiring()
{
    Super::StopFiring();
}

void AAK47::Reload()
{
    if (MagazineState.CurrentAmmo == MagazineState.MaxAmmo || MagazineState.bIsReloading)
    {
        return;
    }

    Super::Reload();
    OnReloadComplete.Broadcast();
}

bool AAK47::CanFire() const
{
    return Super::CanFire() && !MagazineState.bIsReloading;
}

void AAK47::PlayReloadEffects()
{
    Super::PlayReloadEffects();
}

void AAK47::SetupWeaponCollision()
{
    if (WeaponMesh)
    {
        WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    }
}