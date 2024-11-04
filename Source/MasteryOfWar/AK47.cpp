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
		SetFireBehavior(FireBehaviorObj);
	}

	// Initialize AK47-specific configuration
	InitializeWeaponConfig();

	// Load assets
	LoadWeaponAssets();
	
	
    static ConstructorHelpers::FClassFinder<UAmmoWidget> WidgetClassFinder(TEXT("/Game/MofW/Blueprints/WBP_AmmoWidget"));
    if(WidgetClassFinder.Succeeded())
    {
        AmmoWidgetClass = WidgetClassFinder.Class;
    }
}

void AAK47::InitializeWeaponConfig()
{
	AK47Config = FAK47Config(); // This will set all the default values
	Initialize(AK47Config); // Initialize base weapon with AK47 config
}

void AAK47::LoadWeaponAssets()
{
	// Load bullet blueprint
	static ConstructorHelpers::FClassFinder<ABullet> BulletBPClass(TEXT("/Game/Weapons/Blueprints/BP_Bullet"));
	if (BulletBPClass.Succeeded())
	{
		BulletClass = BulletBPClass.Class;
	}

	// Load weapon mesh
	if (UStaticMesh* MeshAsset = Cast<UStaticMesh>(AK47Config.WeaponMeshPath.TryLoad()))
	{
		WeaponModel->SetStaticMesh(MeshAsset);
	}

	// Load effects
	MuzzleFlash = Cast<UParticleSystem>(AK47Config.MuzzleFlashPath.TryLoad());
	EjectedShellEffect = Cast<UParticleSystem>(AK47Config.ShellEjectPath.TryLoad());

	// Load sounds
	FireSound = Cast<USoundBase>(AK47Config.FireSoundPath.TryLoad());
	EmptyMagazineSound = Cast<USoundBase>(AK47Config.EmptyMagSoundPath.TryLoad());
	ReloadSound = Cast<USoundBase>(AK47Config.ReloadSoundPath.TryLoad());
	
	
// Load animations
    ReloadAnimation = Cast<UAnimMontage>(AK47Config.ReloadAnimationPath.TryLoad());
    FireAnimation = Cast<UAnimMontage>(AK47Config.FireAnimationPath.TryLoad());
}

void AAK47::BeginPlay()
{
    Super::BeginPlay();
    
    // Create new fire behavior if needed
    if (!FireBehavior)
    {
        UBulletFireBehavior* FireBehaviorObj = NewObject<UBulletFireBehavior>();
        if (FireBehaviorObj)
        {
            SetFireBehavior(FireBehaviorObj);
        }
    }

    SetupWeaponCollision();
    
    
    // Create AmmoWidget
    if (APlayerController* PC = Cast<APlayerController>(GetOwner()->GetInstigatorController()))
    {
        CreateAmmoWidget(PC);
    } 
}

void AAK47::Fire()
{
    if (!CanFire())
    {
        // Play empty magazine sound
        if (EmptyMagazineSound)
        {
            UGameplayStatics::PlaySoundAtLocation(
                this,
                EmptyMagazineSound,
                GetActorLocation()
            );
        }
        return;
    }

    if (FireBehavior)
    {
        FireBehavior->Fire(this);
        ConsumeAmmo();
        PlayFireEffects();
        ApplyRecoil();
        UpdateAmmoWidget();
    }
}

void AAK47::PlayFireEffects()
{
    // Play fire animation
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
        {
            if (FireAnimation)
            {
                AnimInstance->Montage_Play(FireAnimation);
            }
        }
    }

    // Spawn shell casing effect
    if (EjectedShellEffect)
    {
        FTransform ShellTransform = GetShellEjectTransform();
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            EjectedShellEffect,
            ShellTransform.GetLocation(),
            ShellTransform.GetRotation().Rotator()
        );
    }
    
    // Play fire sound
    if (FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            FireSound,
            GetActorLocation(),
            1.0f,
            1.0f,
            0.0f
        );
    }
}

void AAK47::ApplyRecoil()
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        float RecoilPitch = FMath::RandRange(1.0f, 2.0f) * AK47Config.RecoilStrength;
        float RecoilYaw = FMath::RandRange(-0.5f, 0.5f) * AK47Config.RecoilStrength;
        
        Character->AddControllerPitchInput(-RecoilPitch * 0.05f);
        Character->AddControllerYawInput(RecoilYaw * 0.05f);
    }
}

void AAK47::StartFiring()
{
    // Использует базовую логику из WeaponSystem с учетом режима стрельбы
    Super::StartFiring();
}

void AAK47::StopFiring()
{
    Super::StopFiring();
}

void AAK47::Reload()
{
    if (MagazineState.CurrentAmmo == MagazineState.MaxAmmo)
    {
        return;
    }

    StopFiring();
    PlayReloadEffects();

    // Start reload timer based on animation length
    FTimerHandle ReloadTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        ReloadTimerHandle,
        this,
        &AAK47::ReloadMagazine,
        ReloadAnimation ? ReloadAnimation->GetPlayLength() : 2.0f,
        false
    );
}

void AAK47::PlayReloadEffects()
{
    // Play reload animation
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
        {
            if (ReloadAnimation)
            {
                AnimInstance->Montage_Play(ReloadAnimation);
            }
        }
    }

    // Play reload sound
    if (ReloadSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            ReloadSound,
            GetActorLocation()
        );
    }
}

void AAK47::ReloadMagazine()
{
    Super::ReloadMagazine();
    UpdateAmmoWidget();
    OnReloadComplete.Broadcast();
}

void AAK47::SetupWeaponCollision()
{
    if (WeaponModel)
    {
        WeaponModel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        WeaponModel->SetCollisionResponseToAllChannels(ECR_Ignore);
    }

    if (ACharacter* OwningCharacter = Cast<ACharacter>(GetOwner()))
    {
        FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
        AttachToComponent(OwningCharacter->GetMesh(), AttachRules, FName("WeaponSocket"));
    }
}	