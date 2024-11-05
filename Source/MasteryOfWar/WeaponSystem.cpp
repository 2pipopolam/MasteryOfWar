#include "WeaponSystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

AWeapon::AWeapon()
{
    PrimaryActorTick.bCanEverTick = true;

    WeaponModel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    RootComponent = WeaponModel;

    if (WeaponModel)
    {
        WeaponModel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        WeaponModel->SetCollisionResponseToAllChannels(ECR_Ignore);
    }

    bIsFiring = false;
    CurrentSpread = 0.0f;
}

AWeapon::~AWeapon()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(AutoFireTimerHandle);
    }
}

void AWeapon::Initialize(const FBaseWeaponConfig& InConfig)
{
    Config = InConfig;
    MagazineState.MaxAmmo = Config.MaxAmmo;
    MagazineState.CurrentAmmo = Config.MaxAmmo;
}

void AWeapon::BeginPlay()
{
    Super::BeginPlay();
    CurrentSpread = 0.0f;
    bIsFiring = false;
    
    // Initialize magazine if not done already
    if (MagazineState.MaxAmmo == 0)
    {
        MagazineState.MaxAmmo = Config.MaxAmmo;
        MagazineState.CurrentAmmo = Config.MaxAmmo;
    }
}

void AWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateSpread(DeltaTime);
}

void AWeapon::HandleFireMode()
{
    if (IsAutomaticFireMode())
    {
        // For automatic weapons, continue firing while button is held
        if (bIsFiring)
        {
            Fire();
        }
    }
    else
    {
        // For semi-automatic, fire once per press
        Fire();
    }
}

bool AWeapon::IsAutomaticFireMode() const
{
    return Config.FireMode == EFireMode::Automatic;
}

void AWeapon::ProcessFireInput(bool bPressed)
{
    if (bPressed)
    {
        if (IsAutomaticFireMode())
        {
            StartFiring();
        }
        else
        {
            // Semi-automatic weapons fire once per press
            if (!bIsFiring)
            {
                Fire();
            }
        }
    }
    else
    {
        if (IsAutomaticFireMode())
        {
            StopFiring();
        }
    }
}

void AWeapon::Fire()
{
    if (!CanFire()) return;

    if (FireBehavior)
    {
        FireBehavior->Fire(this);
        ConsumeAmmo();
        UpdateAmmoWidget();
    }
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
                &AWeapon::Fire,
                Config.FireRate,
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

void AWeapon::Reload()
{
    StopFiring();
    ReloadMagazine();
}


bool AWeapon::CanFire() const
{
    return MagazineState.CurrentAmmo > 0 && !MagazineState.bIsReloading;
}


void AWeapon::ConsumeAmmo()
{
    if (MagazineState.CurrentAmmo > 0)
    {
        MagazineState.CurrentAmmo--;
        UpdateAmmoWidget();
    }
}


void AWeapon::ReloadMagazine()
{
    MagazineState.CurrentAmmo = MagazineState.MaxAmmo;
    MagazineState.bIsReloading = false;
    UpdateAmmoWidget();
}


FTransform AWeapon::GetMuzzleTransform() const
{
    if (WeaponModel)
    {
        FTransform SocketTransform = WeaponModel->GetSocketTransform(Config.MuzzleSocketName);
        SocketTransform.AddToTranslation(Config.MuzzleOffset);
        return SocketTransform;
    }
    return GetActorTransform();
}

FTransform AWeapon::GetShellEjectTransform() const
{
    if (WeaponModel)
    {
        return WeaponModel->GetSocketTransform(Config.ShellEjectSocketName);
    }
    return GetActorTransform();
}

FRotator AWeapon::CalculateSpread() const
{
    float TotalSpread = CurrentSpread;
    
    if (IsCharacterMoving())
    {
        TotalSpread += Config.MovementSpread;
    }
    
    if (IsCharacterJumping())
    {
        TotalSpread += Config.JumpingSpread;
    }
    
    float RandomPitch = FMath::RandRange(-TotalSpread, TotalSpread);
    float RandomYaw = FMath::RandRange(-TotalSpread, TotalSpread);
    
    return FRotator(RandomPitch, RandomYaw, 0.0f);
}

void AWeapon::UpdateSpread(float DeltaTime)
{
    if (bIsFiring)
    {
        CurrentSpread = FMath::Min(CurrentSpread + (Config.BaseSpread * DeltaTime), Config.MaxSpread);
    }
    else
    {
        CurrentSpread = FMath::Max(CurrentSpread - (Config.SpreadRecoveryRate * DeltaTime), 0.0f);
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