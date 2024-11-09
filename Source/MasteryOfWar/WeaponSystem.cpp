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
    bIsInRecoil = false;
    CurrentSpread = 0.0f;
    ProjectileSpeed = 8000.0f;
    CurrentRecoilOffset = FVector::ZeroVector;
    CurrentRecoilRotation = FRotator::ZeroRotator;
    RecoilTime = 0.0f;
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

    // Save initial weapon transform
    if (WeaponMesh)
    {
        InitialWeaponLocation = WeaponMesh->GetRelativeLocation();
        InitialWeaponRotation = WeaponMesh->GetRelativeRotation();
        
        // Make sure collision is disabled
        WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
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
    UpdateRecoilState(DeltaTime);
}

void AWeapon::HandleRecoil()
{
    if (!WeaponMesh) return;

    // random recoil
    float RandomX = FMath::RandRange(-WeaponConfig.RecoilRandomness, WeaponConfig.RecoilRandomness);
    float RandomY = FMath::RandRange(-WeaponConfig.RecoilRandomness, WeaponConfig.RecoilRandomness);
    float RandomZ = FMath::RandRange(-WeaponConfig.RecoilRandomness, WeaponConfig.RecoilRandomness);

    // attach recoil
    CurrentRecoilOffset = FVector(
        -WeaponConfig.RecoilOffset * 0.5f,  // move back
        RandomY * 0.3f,                      // random move to side
        WeaponConfig.RecoilOffset            // move up
    );

    // rotate
    CurrentRecoilRotation = FRotator(
        WeaponConfig.RecoilRotation,     // up
        RandomY * 0.5f,                  // to side
        RandomZ * 0.2f                 
    );

    // apply recoil
    FVector NewLocation = InitialWeaponLocation + CurrentRecoilOffset;
    FRotator NewRotation = InitialWeaponRotation + CurrentRecoilRotation;
    WeaponMesh->SetRelativeLocationAndRotation(NewLocation, NewRotation);

    bIsInRecoil = true;
    RecoilTime = 0.0f;
}

void AWeapon::UpdateRecoilState(float DeltaTime)
{
    if (!WeaponMesh || !bIsInRecoil) return;

    RecoilTime += DeltaTime;
    
    // current transform
    FVector CurrentLocation = WeaponMesh->GetRelativeLocation();
    FRotator CurrentRotation = WeaponMesh->GetRelativeRotation();

    // go to initial location
    FVector NewLocation;
    FRotator NewRotation;

    if (bIsFiring)
    {
        // go to initial position before prev shoot
        NewLocation = FMath::VInterpTo(
            CurrentLocation, 
            InitialWeaponLocation + FVector(0, 0, WeaponConfig.RecoilOffset * 0.3f), 
            DeltaTime, 
            WeaponConfig.RecoilRecoverySpeed
        );
        
        NewRotation = FMath::RInterpTo(
            CurrentRotation,
            InitialWeaponRotation + FRotator(WeaponConfig.RecoilRotation * 0.3f, 0, 0),
            DeltaTime,
            WeaponConfig.RecoilRecoverySpeed
        );
    }
    else
    {
        // go to init pos if dont shoot
        NewLocation = FMath::VInterpTo(
            CurrentLocation, 
            InitialWeaponLocation, 
            DeltaTime, 
            WeaponConfig.RecoilRecoverySpeed
        );
        
        NewRotation = FMath::RInterpTo(
            CurrentRotation, 
            InitialWeaponRotation, 
            DeltaTime, 
            WeaponConfig.RecoilRecoverySpeed
        );
    }

    // new transform
    WeaponMesh->SetRelativeLocationAndRotation(NewLocation, NewRotation);

    
    if (!bIsFiring)
    {
        float LocationDiff = FVector::Dist(NewLocation, InitialWeaponLocation);
        float RotationDiff = NewRotation.Equals(InitialWeaponRotation, 0.1f) ? 0.0f : 1.0f;

        if (LocationDiff < 0.1f && RotationDiff < 0.1f)
        {
            bIsInRecoil = false;
            WeaponMesh->SetRelativeLocationAndRotation(InitialWeaponLocation, InitialWeaponRotation);
        }
    }
}

void AWeapon::Fire()
{
    if (!CanFire()) 
    {
        if (EmptyMagazineSound && !MagazineState.bIsReloading)
        {
            UGameplayStatics::PlaySoundAtLocation(
                this, 
                EmptyMagazineSound, 
                GetActorLocation()
            );
        }
        return;
    }

    // Get spawn location
    FTransform MuzzleTransform = GetMuzzleSocketTransform();
    
    // current rotation
    FRotator CurrentAimRotation = GetAdjustedAimDirection().Rotation();
    CurrentAimRotation += CurrentRecoilRotation;
    FVector Direction = CurrentAimRotation.Vector();

    // Spawn bullet with adjusted direction
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
                CurrentAimRotation,
                SpawnParams))
            {
                float Damage = FMath::RandRange(WeaponConfig.MinDamage, WeaponConfig.MaxDamage);
                Bullet->InitializeBullet(Damage, ProjectileSpeed, WeaponConfig.Range);
            }
        }
    }

    // Apply recoil before effects
    HandleRecoil();

    // Play effects and consume ammo
    PlayFireEffects();
    ConsumeAmmo();
    UpdateAmmoWidget();
}





void AWeapon::StartFiring()
{
    if (!bIsFiring)
    {
        if (CanFire())
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
        else if (!MagazineState.bIsReloading && EmptyMagazineSound)
        {
            // empty mag sound
            UGameplayStatics::PlaySoundAtLocation(
                this, 
                EmptyMagazineSound,
                GetActorLocation()
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
    if (bIsFiring)
    {
        if (CanFire())
        {
            Fire();
        }
        else 
        {
            if (!MagazineState.bIsReloading && EmptyMagazineSound)
            {
                UGameplayStatics::PlaySoundAtLocation(
                    this, 
                    EmptyMagazineSound,
                    GetActorLocation()
                );
            }
            StopFiring();
        }
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
    if (!HasValidMuzzleSocket())
        return;

    // Muzzle flash
    if (MuzzleFlashTemplate)
    {
        UParticleSystemComponent* MuzzleFlash = UGameplayStatics::SpawnEmitterAttached(
            MuzzleFlashTemplate,
            WeaponMesh,
            WeaponConfig.MuzzleSocketName,
            FVector::ZeroVector,          
            FRotator::ZeroRotator,  
            MuzzleFlashScale,             
            EAttachLocation::SnapToTarget,
            true,
            EPSCPoolMethod::AutoRelease,
            true
        );

        if (MuzzleFlash)
        {
            MuzzleFlash->SetFloatParameter(TEXT("Lifetime"), MuzzleFlashLifetime);
        }
    }

    // Smoke effect
    if (MuzzleSmokeTemplate)
    {
        FTimerHandle SmokeSpawnTimerHandle;
        GetWorld()->GetTimerManager().SetTimer(
            SmokeSpawnTimerHandle,
            [this]()
            {
                FTransform MuzzleTransform = WeaponMesh->GetSocketTransform(WeaponConfig.MuzzleSocketName);
                FVector WorldSpaceOffset = MuzzleTransform.TransformVector(SmokeSpawnOffset);
                FVector SpawnLocation = MuzzleTransform.GetLocation() + WorldSpaceOffset;

                UParticleSystemComponent* SmokeEffect = UGameplayStatics::SpawnEmitterAtLocation(
                    GetWorld(),
                    MuzzleSmokeTemplate,
                    SpawnLocation,
                    MuzzleTransform.Rotator(),
                    MuzzleSmokeScale
                );

                if (SmokeEffect)
                {
                    SmokeEffect->SetFloatParameter(TEXT("Lifetime"), SmokeLifetime);
                }
            },
            0.1f,
            false
        );
    }

    // Shell ejection
    if (ShellEjectTemplate && WeaponMesh->DoesSocketExist(WeaponConfig.ShellEjectSocketName))
    {
        FTransform ShellTransform = WeaponMesh->GetSocketTransform(WeaponConfig.ShellEjectSocketName);
        FVector EjectionDirection = WeaponMesh->GetRightVector() * ShellEjectOffset.X + 
                                  WeaponMesh->GetUpVector() * ShellEjectOffset.Y +
                                  WeaponMesh->GetForwardVector() * ShellEjectOffset.Z;
        
        ShellTransform.SetLocation(ShellTransform.GetLocation() + EjectionDirection);

        UParticleSystemComponent* ShellEject = UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            ShellEjectTemplate,
            ShellTransform.GetLocation(),
            ShellTransform.GetRotation().Rotator(),
            ShellEjectScale
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
            WeaponMesh->GetSocketLocation(WeaponConfig.MuzzleSocketName)
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
    if (ReloadSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            ReloadSound,
            GetActorLocation()
        );
    }

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