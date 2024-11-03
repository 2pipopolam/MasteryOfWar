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

    MagazineState.MaxAmmo = 30;
    MagazineState.CurrentAmmo = 30;

}

AWeapon::~AWeapon()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(AutoFireTimerHandle);
    }
}

void AWeapon::BeginPlay()
{
    Super::BeginPlay();
    CurrentSpread = 0.0f;
    bIsFiring = false;
}

void AWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateSpread(DeltaTime);
}

void AWeapon::Fire()
{
    if (!CanFire()) return;

    if (FireBehavior)
    {
        FireBehavior->Fire(this);
        ConsumeAmmo();
    }
}

void AWeapon::StartFiring()
{
    if (!bIsFiring && CanFire())
    {
        bIsFiring = true;
        Fire();
        GetWorld()->GetTimerManager().SetTimer(
            AutoFireTimerHandle,
            this,
            &AWeapon::Fire,
            FireRate,
            true
        );
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
    return MagazineState.CurrentAmmo > 0;
}

void AWeapon::ConsumeAmmo()
{
    if (MagazineState.CurrentAmmo > 0)
    {
        MagazineState.CurrentAmmo--;
    }
}

void AWeapon::ReloadMagazine()
{
    MagazineState.CurrentAmmo = MagazineState.MaxAmmo;
}

FTransform AWeapon::GetMuzzleTransform() const
{
    if (WeaponModel)
    {
        FTransform SocketTransform = WeaponModel->GetSocketTransform(MuzzleSocketName);
        SocketTransform.AddToTranslation(MuzzleOffset);
        return SocketTransform;
    }
    return GetActorTransform();
}

FTransform AWeapon::GetShellEjectTransform() const
{
    if (WeaponModel)
    {
        return WeaponModel->GetSocketTransform(ShellEjectSocketName);
    }
    return GetActorTransform();
}

FRotator AWeapon::CalculateSpread() const
{
    float TotalSpread = CurrentSpread;
    
    if (IsCharacterMoving())
    {
        TotalSpread += MovementSpread;
    }
    
    if (IsCharacterJumping())
    {
        TotalSpread += JumpingSpread;
    }
    
    float RandomPitch = FMath::RandRange(-TotalSpread, TotalSpread);
    float RandomYaw = FMath::RandRange(-TotalSpread, TotalSpread);
    
    return FRotator(RandomPitch, RandomYaw, 0.0f);
}

void AWeapon::UpdateSpread(float DeltaTime)
{
    if (bIsFiring)
    {
        CurrentSpread = FMath::Min(CurrentSpread + (BaseSpread * DeltaTime), MaxSpread);
    }
    else
    {
        CurrentSpread = FMath::Max(CurrentSpread - (SpreadRecoveryRate * DeltaTime), 0.0f);
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