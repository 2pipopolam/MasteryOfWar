#include "WeaponSystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Particles/ParticleSystemComponent.h"
#include "DrawDebugHelpers.h"
#include "MasteryOfWarCharacter.h"

UCameraRecoilComponent::UCameraRecoilComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentPatternIndex = 0;
    bIsRecoilActive = false;
    bIsInSmoothRecovery = false;
    TotalRecoilOffset = FVector2D::ZeroVector;
    RecoilRecoveryOffset = FVector2D::ZeroVector;
    TimeSinceLastRecoil = 0.0f;
    TargetCamera = nullptr;
}

void UCameraRecoilComponent::BeginPlay()
{
    Super::BeginPlay();
    
    TimeSinceLastRecoil = 0.0f;
    bIsInSmoothRecovery = false;
    
    if (!TargetCamera)
    {
        TargetCamera = GetCharacterCamera();
        //UE_LOG(LogTemp, Warning, TEXT("CameraRecoilComponent BeginPlay, Camera found: %s"), 
            //TargetCamera ? TEXT("Yes") : TEXT("No"));
    }
}

void UCameraRecoilComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsRecoilActive)
    {
        TimeSinceLastRecoil += DeltaTime;

        if (TimeSinceLastRecoil >= CurrentPattern.RecoveryDelay &&
            !TotalRecoilOffset.IsNearlyZero(CurrentPattern.MinRecoilForRecovery))
        {
            if (!bIsInSmoothRecovery)
            {
                bIsInSmoothRecovery = true;
                SmoothRecoveryStartPosition = TotalRecoilOffset;
            }
            
            HandleSmoothRecovery(DeltaTime);
        }
        else if (!bIsInSmoothRecovery && !TotalRecoilOffset.IsNearlyZero())
        {
            RecoverFromRecoil(DeltaTime);
        }
    }
}

void UCameraRecoilComponent::HandleSmoothRecovery(float DeltaTime)
{
    if (!bIsInSmoothRecovery || !TargetCamera)
        return;

    FVector2D TargetPosition = FVector2D::ZeroVector;
    FVector2D NewPosition = FMath::Vector2DInterpTo(
        TotalRecoilOffset,
        TargetPosition,
        DeltaTime,
        CurrentPattern.SmoothRecoverySpeed
    );
    
    FVector2D Delta = NewPosition - TotalRecoilOffset;
    
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()->GetOwner()))
    {
        if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
        {
            PC->AddPitchInput(Delta.Y);
            PC->AddYawInput(Delta.X);
        }
    }

    TotalRecoilOffset = NewPosition;
    
    if (TotalRecoilOffset.IsNearlyZero(0.01f))
    {
        bIsInSmoothRecovery = false;
        TotalRecoilOffset = FVector2D::ZeroVector;
        RecoilRecoveryOffset = FVector2D::ZeroVector;
    }
}

UCameraComponent* UCameraRecoilComponent::GetCharacterCamera() const
{
    if (AActor* Owner = GetOwner())
    {
        if (APawn* OwnerPawn = Cast<APawn>(Owner->GetOwner()))
        {
            if (AMasteryOfWarCharacter* Character = Cast<AMasteryOfWarCharacter>(OwnerPawn))
            {
                return Character->GetFollowCamera();
            }
        }
    }
    return nullptr;
}

void UCameraRecoilComponent::ApplyRecoil()
{
    if (!CurrentPattern.PatternPoints.IsValidIndex(CurrentPatternIndex))
    {
        CurrentPatternIndex = 0;
    }

    if (!CurrentPattern.PatternPoints.Num())
    {
        //UE_LOG(LogTemp, Error, TEXT("No recoil pattern points!"));
        return;
    }

    bIsRecoilActive = true;
    TimeSinceLastRecoil = 0.0f;
    bIsInSmoothRecovery = false;
    
    FVector2D RecoilPoint = GetNextPatternPoint();
    
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()->GetOwner()))
    {
        if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
        {
            float PitchInput = -(RecoilPoint.Y * CurrentPattern.RecoilStrength);
            float YawInput = RecoilPoint.X * CurrentPattern.RecoilStrength;
            
            PC->AddPitchInput(PitchInput);
            PC->AddYawInput(YawInput);
            
            TotalRecoilOffset += FVector2D(YawInput, PitchInput);
            
            //UE_LOG(LogTemp, Warning, TEXT("Applied recoil - PitchInput: %f, YawInput: %f"), 
                //PitchInput, YawInput);
        }
    }

    CurrentPatternIndex++;
}

void UCameraRecoilComponent::ResetRecoil()
{
    //UE_LOG(LogTemp, Warning, TEXT("ResetRecoil called"));
    bIsRecoilActive = false;
    bIsInSmoothRecovery = false;
    CurrentPatternIndex = 0;
    RecoilRecoveryOffset = TotalRecoilOffset;
}

void UCameraRecoilComponent::SetRecoilPattern(const FCameraRecoilPattern& NewPattern)
{
    //UE_LOG(LogTemp, Warning, TEXT("SetRecoilPattern called. Points: %d"), NewPattern.PatternPoints.Num());
    CurrentPattern = NewPattern;
    ResetRecoil();
}

void UCameraRecoilComponent::RecoverFromRecoil(float DeltaTime)
{
    if (RecoilRecoveryOffset.IsNearlyZero())
        return;

    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()->GetOwner()))
    {
        if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
        {
            FVector2D RecoveryDirection = -RecoilRecoveryOffset.GetSafeNormal();
            
            float RecoveryMagnitude = FMath::Min(
                RecoilRecoveryOffset.Size() * CurrentPattern.RecoverySpeed * DeltaTime * 0.5f,
                RecoilRecoveryOffset.Size()
            );
            
            FVector2D Recovery = RecoveryDirection * RecoveryMagnitude;
            
            PC->AddPitchInput(Recovery.Y);
            PC->AddYawInput(Recovery.X);
            
            RecoilRecoveryOffset += Recovery;
            TotalRecoilOffset += Recovery;
            
            if (RecoilRecoveryOffset.Size() < 0.01f)
            {
                RecoilRecoveryOffset = FVector2D::ZeroVector;
                TotalRecoilOffset = FVector2D::ZeroVector;
            }
        }
    }
}

FVector2D UCameraRecoilComponent::GetNextPatternPoint() const
{
    if (!CurrentPattern.PatternPoints.IsValidIndex(CurrentPatternIndex))
    {
        return FVector2D::ZeroVector;
    }

    FVector2D BasePoint = CurrentPattern.PatternPoints[CurrentPatternIndex];
    
    float RandomX = FMath::RandRange(-CurrentPattern.RandomDeviation, CurrentPattern.RandomDeviation);
    float RandomY = FMath::RandRange(-CurrentPattern.RandomDeviation, CurrentPattern.RandomDeviation);
    
    return BasePoint + FVector2D(RandomX, RandomY);
}

// Weapon implementation
AWeapon::AWeapon()
{
    PrimaryActorTick.bCanEverTick = true;

    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    RootComponent = WeaponMesh;

    CameraRecoilComponent = CreateDefaultSubobject<UCameraRecoilComponent>(TEXT("CameraRecoil"));

    bIsFiring = false;
    bIsInRecoil = false;
    CurrentSpread = 0.0f;
    ProjectileSpeed = 8000.0f;
    CurrentRecoilOffset = FVector::ZeroVector;
    CurrentRecoilRotation = FRotator::ZeroRotator;
    RecoilTime = 0.0f;

    MuzzleFlashScale = FVector(0.05f);
    ShellEjectScale = FVector(0.3f);
    MuzzleFlashOffset = FVector::ZeroVector;
    ShellEjectOffset = FVector(10.0f, 5.0f, 0.0f);
    MuzzleFlashLifetime = 0.2f;
    MuzzleSmokeScale = FVector(1.0f);
    SmokeLifetime = 1.0f;
    SmokeSpawnOffset = FVector::ZeroVector;
}


FTransform AWeapon::GetBulletSpawnTransform() const
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCameraComponent* Camera = Character->FindComponentByClass<UCameraComponent>())
        {
            FVector CameraLocation = Camera->GetComponentLocation();
            FRotator CameraRotation = Camera->GetComponentRotation();
            
            float SpreadX = 0.0f;
            float SpreadY = 0.0f;
            
            bool bIsCrouching = Character->bIsCrouched;
            bool bIsJumping = Character->GetMovementComponent()->IsFalling();
            bool bIsMoving = !Character->GetVelocity().IsNearlyZero();
            bool bIsWalking = false;
            
            if (AMasteryOfWarCharacter* MOWCharacter = Cast<AMasteryOfWarCharacter>(Character))
            {
                bIsWalking = MOWCharacter->IsWalking();
            }

            if (bIsJumping)
            {
                SpreadX = FMath::RandRange(-WeaponConfig.SpreadConfig.JumpSpreadMax, WeaponConfig.SpreadConfig.JumpSpreadMax);
                SpreadY = FMath::RandRange(-WeaponConfig.SpreadConfig.JumpSpreadMax, WeaponConfig.SpreadConfig.JumpSpreadMax);
            }
            else if (bIsMoving)
            {
                if (bIsCrouching)
                {
                    SpreadX = FMath::RandRange(-WeaponConfig.SpreadConfig.CrouchMoveSpreadMax, WeaponConfig.SpreadConfig.CrouchMoveSpreadMax);
                    SpreadY = FMath::RandRange(-WeaponConfig.SpreadConfig.CrouchMoveSpreadMax, WeaponConfig.SpreadConfig.CrouchMoveSpreadMax);
                }
                else if (bIsWalking)
                {
                    SpreadX = FMath::RandRange(-WeaponConfig.SpreadConfig.WalkSpreadMax, WeaponConfig.SpreadConfig.WalkSpreadMax);
                    SpreadY = FMath::RandRange(-WeaponConfig.SpreadConfig.WalkSpreadMax, WeaponConfig.SpreadConfig.WalkSpreadMax);
                }
                else
                {
                    SpreadX = FMath::RandRange(-WeaponConfig.SpreadConfig.RunSpreadMax, WeaponConfig.SpreadConfig.RunSpreadMax);
                    SpreadY = FMath::RandRange(-WeaponConfig.SpreadConfig.RunSpreadMax, WeaponConfig.SpreadConfig.RunSpreadMax);
                }
                
                float SpeedFactor = (Character->GetVelocity().Size() / 500.0f) * WeaponConfig.SpreadConfig.SpeedSpreadMultiplier;
                SpreadX *= (1.0f + SpeedFactor);
                SpreadY *= (1.0f + SpeedFactor);
            }
            else if (bIsCrouching)
            {
                SpreadX = FMath::RandRange(-WeaponConfig.SpreadConfig.BaseSpread, WeaponConfig.SpreadConfig.BaseSpread) * 
                         WeaponConfig.SpreadConfig.CrouchSpreadMultiplier;
                SpreadY = FMath::RandRange(-WeaponConfig.SpreadConfig.BaseSpread, WeaponConfig.SpreadConfig.BaseSpread) * 
                         WeaponConfig.SpreadConfig.CrouchSpreadMultiplier;
            }
            else
            {
                SpreadX = FMath::RandRange(-WeaponConfig.SpreadConfig.BaseSpread, WeaponConfig.SpreadConfig.BaseSpread);
                SpreadY = FMath::RandRange(-WeaponConfig.SpreadConfig.BaseSpread, WeaponConfig.SpreadConfig.BaseSpread);
            }
            
            if (CurrentSpread > 0.0f)
            {
                float AdditionalSpread = FMath::Min(CurrentSpread, WeaponConfig.SpreadConfig.MaxSpreadIncrease);
                SpreadX += FMath::RandRange(-AdditionalSpread, AdditionalSpread);
                SpreadY += FMath::RandRange(-AdditionalSpread, AdditionalSpread);
            }
            
            // spread to shot direction
            FRotator SpreadRotation = CameraRotation;
            SpreadRotation.Pitch += SpreadY;
            SpreadRotation.Yaw += SpreadX;
            
            FVector SpawnOffset = SpreadRotation.Vector() * 50.0f;
            FVector SpawnLocation = CameraLocation + SpawnOffset;
            
            return FTransform(SpreadRotation, SpawnLocation);
        }
    }
    
    return GetMuzzleSocketTransform();
}


void AWeapon::BeginPlay()
{
    Super::BeginPlay();

    if (MagazineState.MaxAmmo == 0)
    {
        MagazineState.MaxAmmo = WeaponConfig.MaxAmmo;
        MagazineState.CurrentAmmo = WeaponConfig.MaxAmmo;
    }

    if (WeaponMesh)
    {
        InitialWeaponLocation = WeaponMesh->GetRelativeLocation();
        InitialWeaponRotation = WeaponMesh->GetRelativeRotation();
        WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    }

    if (CameraRecoilComponent)
    {
        if (!CameraRecoilComponent->GetCharacterCamera())
        {
            if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
            {
                if (AMasteryOfWarCharacter* Character = Cast<AMasteryOfWarCharacter>(OwnerPawn))
                {
                    CameraRecoilComponent->SetTargetCamera(Character->GetFollowCamera());
                }
            }
        }
    }

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

    FTransform SpawnTransform = GetBulletSpawnTransform();
    
    if (UWorld* World = GetWorld())
    {
        if (BulletClass)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            SpawnParams.Instigator = Cast<APawn>(GetOwner());

            if (ABullet* Bullet = World->SpawnActor<ABullet>(
                BulletClass, 
                SpawnTransform.GetLocation(),
                SpawnTransform.GetRotation().Rotator(),
                SpawnParams))
            {
                int32 Damage = WeaponConfig.DamageConfig.BaseDamage;
                Bullet->InitializeBullet(Damage, ProjectileSpeed, WeaponConfig.Range);
            }
        }
    }

    HandleRecoil();
    
    if (CameraRecoilComponent)
    {
        CameraRecoilComponent->ApplyRecoil();
    }

    PlayFireEffects();
    ConsumeAmmo();
    UpdateAmmoWidget();
}

void AWeapon::HandleRecoil()
{
    if (!WeaponMesh) return;

    float RandomX = FMath::RandRange(-WeaponConfig.RecoilRandomness, WeaponConfig.RecoilRandomness);
    float RandomY = FMath::RandRange(-WeaponConfig.RecoilRandomness, WeaponConfig.RecoilRandomness);
    float RandomZ = FMath::RandRange(-WeaponConfig.RecoilRandomness, WeaponConfig.RecoilRandomness);

    CurrentRecoilOffset = FVector(
        -WeaponConfig.RecoilOffset * 0.5f,
        RandomY * 0.3f,
        WeaponConfig.RecoilOffset
    );

    CurrentRecoilRotation = FRotator(
        WeaponConfig.RecoilRotation,
        RandomY * 0.5f,
        RandomZ * 0.2f
    );

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
    
    FVector CurrentLocation = WeaponMesh->GetRelativeLocation();
    FRotator CurrentRotation = WeaponMesh->GetRelativeRotation();
    FVector NewLocation;
    FRotator NewRotation;

    if (bIsFiring)
    {
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
        
        if (CameraRecoilComponent)
        {
            CameraRecoilComponent->ResetRecoil();
        }
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

FVector AWeapon::GetAdjustedAimDirection() const
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        if (UCameraComponent* Camera = Character->FindComponentByClass<UCameraComponent>())
        {
            return Camera->GetForwardVector();
        }
    }
    return GetActorForwardVector();
}


void AWeapon::UpdateSpread(float DeltaTime)
{   
    if (bIsFiring)
    {
        CurrentSpread += WeaponConfig.SpreadConfig.SpreadIncreasePerShot * DeltaTime;
        
        if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
        {
            bool bIsCrouching = Character->bIsCrouched;
            bool bIsJumping = Character->GetMovementComponent()->IsFalling();
            bool bIsMoving = !Character->GetVelocity().IsNearlyZero();
            bool bIsWalking = false;
            
            if (AMasteryOfWarCharacter* MOWCharacter = Cast<AMasteryOfWarCharacter>(Character))
            {
                bIsWalking = MOWCharacter->IsWalking();
            }
            // spread depends on movement type
            if (bIsJumping)
            {
                CurrentSpread = FMath::Min(CurrentSpread, WeaponConfig.SpreadConfig.JumpSpreadMax);
            }
            else if (bIsMoving)
            {
                if (bIsCrouching)
                {
                    CurrentSpread = FMath::Min(CurrentSpread, WeaponConfig.SpreadConfig.CrouchMoveSpreadMax);
                }
                else if (bIsWalking)
                {
                    CurrentSpread = FMath::Min(CurrentSpread, WeaponConfig.SpreadConfig.WalkSpreadMax);
                }
                else
                {
                    CurrentSpread = FMath::Min(CurrentSpread, WeaponConfig.SpreadConfig.RunSpreadMax);
                }

                // speed factor for shooting
                float SpeedFactor = (Character->GetVelocity().Size() / 500.0f) * 
                                   WeaponConfig.SpreadConfig.SpeedSpreadMultiplier;
                CurrentSpread *= (1.0f + SpeedFactor);
            }
        }

        // limit spread
        CurrentSpread = FMath::Min(CurrentSpread, WeaponConfig.SpreadConfig.MaxSpreadIncrease);
    }
    else
    {
        // restore accur
        CurrentSpread = FMath::Max(
            CurrentSpread - (WeaponConfig.SpreadConfig.SpreadRecoveryRate * DeltaTime),
            WeaponConfig.SpreadConfig.BaseSpread
        );
    }

    //UE_LOG(LogTemp, VeryVerbose, TEXT("Weapon: %s, CurrentSpread: %f"), 
        //*GetName(), CurrentSpread);
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

    if (MuzzleFlashTemplate)
    {
        UParticleSystemComponent* MuzzleFlash = UGameplayStatics::SpawnEmitterAttached(
            MuzzleFlashTemplate,
            WeaponMesh,
            WeaponConfig.MuzzleSocketName,
            MuzzleFlashOffset,          
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

    if (FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            FireSound,
            WeaponMesh->GetSocketLocation(WeaponConfig.MuzzleSocketName)
        );
    }

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