#include "WeaponSystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Particles/ParticleSystemComponent.h"
#include "DrawDebugHelpers.h"

AWeapon::AWeapon()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create weapon mesh component
    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    RootComponent = WeaponMesh;

    // Setup collision
    if (WeaponMesh)
    {
        WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    }

    // Initialize default values
    bIsFiring = false;
    CurrentSpread = 0.0f;
    ProjectileSpeed = 8000.0f;
}

void AWeapon::BeginPlay()
{
    Super::BeginPlay();

    // Initialize magazine if not set in editor
    if (MagazineState.MaxAmmo == 0)
    {
        MagazineState.MaxAmmo = WeaponConfig.MaxAmmo;
        MagazineState.CurrentAmmo = WeaponConfig.MaxAmmo;
    }

    // Validate weapon setup
    if (HasValidMuzzleSocket())
    {
        UE_LOG(LogTemp, Log, TEXT("Weapon %s initialized with valid muzzle socket"), *GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Weapon %s missing muzzle socket configuration!"), *GetName());
    }

    // Initialize AmmoWidget if we have a player controller
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
        {
            CreateAmmoWidget(PC);
        }
    }
}

void AWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateSpread(DeltaTime);
}

bool AWeapon::HasValidMuzzleSocket() const
{
    return WeaponMesh && WeaponMesh->DoesSocketExist(WeaponConfig.MuzzleSocketName);
}

FTransform AWeapon::GetMuzzleSocketTransform() const
{
    if (HasValidMuzzleSocket())
    {
        return WeaponMesh->GetSocketTransform(WeaponConfig.MuzzleSocketName);
    }
    return GetActorTransform();
}

FTransform AWeapon::GetShellEjectSocketTransform() const
{
    if (WeaponMesh && WeaponMesh->DoesSocketExist(WeaponConfig.ShellEjectSocketName))
    {
        return WeaponMesh->GetSocketTransform(WeaponConfig.ShellEjectSocketName);
    }
    return GetActorTransform();
}

void AWeapon::Fire()
{
    if (!CanFire()) 
    {
        if (EmptyMagazineSound && !MagazineState.bIsReloading)
        {
            UGameplayStatics::PlaySoundAtLocation(this, EmptyMagazineSound, GetActorLocation());
        }
        return;
    }

    // Get spawn location and direction
    FTransform MuzzleTransform = GetMuzzleSocketTransform();
    FVector Direction = GetAdjustedAimDirection();

    // Spawn bullet
    if (UWorld* World = GetWorld())
    {
        if (BulletClass)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            SpawnParams.Instigator = Cast<APawn>(GetOwner());

            if (ABullet* Bullet = World->SpawnActor<ABullet>(
                BulletClass, 
                MuzzleTransform.GetLocation(),
                Direction.Rotation(),
                SpawnParams))
            {
                float Damage = FMath::RandRange(WeaponConfig.MinDamage, WeaponConfig.MaxDamage);
                Bullet->InitializeBullet(Damage, ProjectileSpeed, WeaponConfig.Range);
            }
        }
    }

    // Play effects and consume ammo
    PlayFireEffects();
    ConsumeAmmo();
    UpdateAmmoWidget();
}

void AWeapon::StartFiring()
{
    if (!bIsFiring && CanFire())
    {
        bIsFiring = true;
        Fire();
        
        if (IsAutomaticFireMode())
        {
            GetWorld()->GetTimerManager().SetTimer(
                AutoFireTimerHandle,
                this,
                &AWeapon::HandleAutoFire,
                WeaponConfig.FireRate,
                true
            );
        }
    }
}

void AWeapon::StopFiring()
{
    if (bIsFiring)
    {
        bIsFiring = false;
        GetWorld()->GetTimerManager().ClearTimer(AutoFireTimerHandle);
    }
}

void AWeapon::HandleAutoFire()
{
    if (bIsFiring && CanFire())
    {
        Fire();
    }
    else
    {
        StopFiring();
    }
}

void AWeapon::Reload()
{
    if (MagazineState.CurrentAmmo == MagazineState.MaxAmmo || MagazineState.bIsReloading)
    {
        return;
    }

    StopFiring();
    MagazineState.bIsReloading = true;
    PlayReloadEffects();

    // Set timer for reload completion
    FTimerHandle ReloadTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        ReloadTimerHandle,
        [this]()
        {
            MagazineState.CurrentAmmo = MagazineState.MaxAmmo;
            MagazineState.bIsReloading = false;
            UpdateAmmoWidget();
        },
        WeaponConfig.ReloadTime,
        false
    );
}

bool AWeapon::CanFire() const
{
    return MagazineState.CurrentAmmo > 0 && 
           !MagazineState.bIsReloading && 
           HasValidMuzzleSocket();
}

void AWeapon::ConsumeAmmo()
{
    if (MagazineState.CurrentAmmo > 0)
    {
        MagazineState.CurrentAmmo--;
    }
}

bool AWeapon::IsAutomaticFireMode() const
{
    return WeaponConfig.FireMode == EFireMode::Automatic;
}

FVector AWeapon::GetAdjustedAimDirection() const
{
    FVector AimDirection = GetActorForwardVector();

    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCameraComponent* Camera = Character->FindComponentByClass<UCameraComponent>())
        {
            AimDirection = Camera->GetForwardVector();
        }
    }

    FRotator SpreadRotator = CalculateSpread();
    return AimDirection.RotateAngleAxis(SpreadRotator.Pitch, GetActorRightVector())
                      .RotateAngleAxis(SpreadRotator.Yaw, GetActorUpVector());
}

FRotator AWeapon::CalculateSpread() const
{
    float TotalSpread = CurrentSpread;
    
    if (IsCharacterMoving())
    {
        TotalSpread += WeaponConfig.MovementSpread;
    }
    
    if (IsCharacterJumping())
    {
        TotalSpread += WeaponConfig.JumpingSpread;
    }
    
    float RandomPitch = FMath::RandRange(-TotalSpread, TotalSpread);
    float RandomYaw = FMath::RandRange(-TotalSpread, TotalSpread);
    
    return FRotator(RandomPitch, RandomYaw, 0.0f);
}

void AWeapon::UpdateSpread(float DeltaTime)
{
    if (bIsFiring)
    {
        CurrentSpread = FMath::Min(CurrentSpread + (WeaponConfig.BaseSpread * DeltaTime), 
                                  WeaponConfig.MaxSpread);
    }
    else
    {
        CurrentSpread = FMath::Max(CurrentSpread - (WeaponConfig.SpreadRecoveryRate * DeltaTime), 
                                  0.0f);
    }
}

bool AWeapon::IsCharacterMoving() const
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        return !Character->GetVelocity().IsNearlyZero();
    }
    return false;
}

bool AWeapon::IsCharacterJumping() const
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        return Character->GetCharacterMovement()->IsFalling();
    }
    return false;
}




void AWeapon::PlayFireEffects()
{
    // Muzzle flash
    if (MuzzleFlashTemplate && HasValidMuzzleSocket())
    {
        FTransform EmitterTransform = GetMuzzleSocketTransform();
        EmitterTransform.SetScale3D(FVector(0.5f));

        UParticleSystemComponent* MuzzleFlash = UGameplayStatics::SpawnEmitterAttached(
            MuzzleFlashTemplate,
            WeaponMesh,
            WeaponConfig.MuzzleSocketName,
            EmitterTransform.GetLocation(),
            EmitterTransform.GetRotation().Rotator(),
            EmitterTransform.GetScale3D(),
            EAttachLocation::SnapToTarget
        );

        if (MuzzleFlash)
        {
            // Установка размера
            MuzzleFlash->SetRelativeScale3D(FVector(0.5f));
            
            // Параметры частиц
            MuzzleFlash->SetFloatParameter(TEXT("Size"), 0.5f);
            MuzzleFlash->SetFloatParameter(TEXT("Lifetime"), 0.2f);
        }
    }

    // Shell ejection
    if (ShellEjectTemplate)
    {
        FTransform ShellTransform = GetShellEjectSocketTransform();
        ShellTransform.SetScale3D(FVector(0.3f));

        UParticleSystemComponent* ShellEject = UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            ShellEjectTemplate,
            ShellTransform
        );

        if (ShellEject)
        {
            ShellEject->SetFloatParameter(TEXT("SpawnRate"), 1.0f);
        }
    }

    // Fire sound
    if (FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            FireSound,
            GetActorLocation()
        );
    }

    // Fire animation
    if (FireAnimation)
    {
        if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
        {
            if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
            {
                AnimInstance->Montage_Play(FireAnimation);
            }
        }
    }
}




void AWeapon::PlayReloadEffects()
{
    // Reload sound
    if (ReloadSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            ReloadSound,
            GetActorLocation()
        );
    }

    // Reload animation
    if (ReloadAnimation)
    {
        if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
        {
            if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
            {
                AnimInstance->Montage_Play(ReloadAnimation);
            }
        }
    }
}

void AWeapon::CreateAmmoWidget(APlayerController* PC)
{
    if (!PC || !AmmoWidgetClass) return;

    if (!AmmoWidget)
    {
        AmmoWidget = CreateWidget<UAmmoWidget>(PC, AmmoWidgetClass);
        if (AmmoWidget)
        {
            AmmoWidget->AddToViewport(1);
            UpdateAmmoWidget();
        }
    }
}

void AWeapon::UpdateAmmoWidget()
{
    if (AmmoWidget)
    {
        AmmoWidget->UpdateAmmoCount(MagazineState.CurrentAmmo, MagazineState.MaxAmmo);
    }
}