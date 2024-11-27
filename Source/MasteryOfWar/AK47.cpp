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
    AK47Config = FAK47Config();
    WeaponConfig = AK47Config;
    
    //UE_LOG(LogTemp, Warning, TEXT("AK47 Config Initialized - Base Damage: %d"), 
           //AK47Config.DamageConfig.BaseDamage);
}

void AAK47::LoadWeaponAssets()
{
    // upload static mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Game/Weapons/AK47/ak-47"));
    if (MeshAsset.Succeeded() && WeaponMesh)
    {
        WeaponMesh->SetStaticMesh(MeshAsset.Object);
    }

    //effects
    static ConstructorHelpers::FObjectFinder<UParticleSystem> MuzzleFlashFX(TEXT("/Game/Weapons/Particles/Fire"));
    if (MuzzleFlashFX.Succeeded())
    {
        MuzzleFlashTemplate = MuzzleFlashFX.Object;
    }
/*
    static ConstructorHelpers::FObjectFinder<UParticleSystem> ShellEjectFX(TEXT("/Game/Weapons/Particles/P_ShellEject_AK47"));
    if (ShellEjectFX.Succeeded())
    {
        EjectedShellEffect = ShellEjectFX.Object;
    }
*/
    // sounds
    static ConstructorHelpers::FObjectFinder<USoundBase> FireSFX(TEXT("/Game/Weapons/Sounds/single_shoot_ak47"));
    if (FireSFX.Succeeded())
    {
        FireSound = FireSFX.Object;
    }

    static ConstructorHelpers::FObjectFinder<USoundBase> EmptySFX(TEXT("/Game/Weapons/Sounds/empty_mag_sound"));
    if (EmptySFX.Succeeded())
    {
        EmptyMagazineSound = EmptySFX.Object;
    }

    static ConstructorHelpers::FObjectFinder<USoundBase> ReloadSFX(TEXT("/Game/Weapons/Sounds/reloading_ak47"));
    if (ReloadSFX.Succeeded())
    {
        ReloadSound = ReloadSFX.Object;
    }

    // animations
    /*
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
    */

    // bullet
    static ConstructorHelpers::FClassFinder<ABullet> BulletBPClass(TEXT("/Game/Weapons/Blueprints/BP_Bullet"));
    if (BulletBPClass.Succeeded())
    {
        BulletClass = BulletBPClass.Class;
    }

    // widget
    static ConstructorHelpers::FClassFinder<UAmmoWidget> WidgetClassFinder(TEXT("/Game/MofW/Blueprints/WBP_AmmoWidget"));
    if(WidgetClassFinder.Succeeded())
    {
        AmmoWidgetClass = WidgetClassFinder.Class;
    }
}


void AAK47::BeginPlay()
{
    Super::BeginPlay();
    SetupWeaponCollision();
    
    // camera check and init 
    if (CameraRecoilComponent)
    {
        // does pattern have some points? 
        ensure(AK47Config.RecoilPattern.PatternPoints.Num() > 0);
        
        CameraRecoilComponent->SetRecoilPattern(AK47Config.RecoilPattern);
        
        // check owner and controller
        if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
        {
            if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
            {
                //UE_LOG(LogTemp, Warning, TEXT("Found valid controller for recoil"));
                
                if (UCameraComponent* Camera = OwnerPawn->FindComponentByClass<UCameraComponent>())
                {
                    CameraRecoilComponent->SetTargetCamera(Camera);
                    //UE_LOG(LogTemp, Warning, TEXT("Camera set for recoil"));
                }
            }
        }

        /*
        UE_LOG(LogTemp, Warning, TEXT("Recoil pattern set with %d points, strength: %f"), 
            AK47Config.RecoilPattern.PatternPoints.Num(),
            AK47Config.RecoilPattern.RecoilStrength);
        */
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("CameraRecoilComponent is null in AK47 BeginPlay!"));
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