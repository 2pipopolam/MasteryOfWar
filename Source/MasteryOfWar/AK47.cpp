#include "AK47.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "BulletFireBehavior.h"


AAK47::AAK47()
{
    UBulletFireBehavior* FireBehaviorObj = NewObject<UBulletFireBehavior>();
 
    if (FireBehaviorObj)
    {
        SetFireBehavior(FireBehaviorObj);
    }



    static ConstructorHelpers::FClassFinder<ABullet> BulletBPClass(TEXT("/Game/Weapons/Blueprints/BP_Bullet"));
    
    
    if (BulletBPClass.Succeeded())
    {
        BulletClass = BulletBPClass.Class;  // Просто присваиваем значение
        UE_LOG(LogTemp, Warning, TEXT("Successfully loaded bullet class"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load bullet class BP_Bullet"));
    }

    
    // Уникальные параметры АК-47
    FireRate = 0.1f;         // 600 RPM
    MinDamage = 25.0f;      // Минимальный урон
    MaxDamage = 45.0f;      // Максимальный урон
    Range = 8000.0f;        // Дальность стрельбы
    
    // Параметры разброса
    BaseSpread = 0.2f;          // Базовый разброс
    MovementSpread = 1.5f;      // Разброс при движении
    JumpingSpread = 3.0f;       // Разброс при прыжке
    SpreadRecoveryRate = 0.5f;  // Скорость восстановления точности
    MaxSpread = 4.0f; 
    
    
    MagazineState.MaxAmmo = 30;
    MagazineState.CurrentAmmo = 30;

    // Загрузка ассетов
    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Game/Weapons/Meshes/AK47_Mesh"));
    if (MeshAsset.Succeeded())
    {
        WeaponModel->SetStaticMesh(MeshAsset.Object);
    }

    static ConstructorHelpers::FObjectFinder<UParticleSystem> MuzzleFlashAsset(TEXT("/Game/Effects/Particles/P_MuzzleFlash_AK47"));
    if (MuzzleFlashAsset.Succeeded())
    {
        MuzzleFlash = MuzzleFlashAsset.Object;
    }

    static ConstructorHelpers::FObjectFinder<UParticleSystem> ShellEjectAsset(TEXT("/Game/Effects/Particles/P_ShellEject_AK47"));
    if (ShellEjectAsset.Succeeded())
    {
        EjectedShellEffect = ShellEjectAsset.Object;
    }

    static ConstructorHelpers::FObjectFinder<USoundBase> FireSoundAsset(TEXT("/Game/Weapons/Sounds/single_shoot_ak47"));
    if (FireSoundAsset.Succeeded())
    {
        FireSound = FireSoundAsset.Object;
    }

    static ConstructorHelpers::FObjectFinder<USoundBase> EmptyMagSoundAsset(TEXT("/Game/Sounds/Weapons/S_EmptyMag"));
    if (EmptyMagSoundAsset.Succeeded())
    {
        EmptyMagazineSound = EmptyMagSoundAsset.Object;
    }

    static ConstructorHelpers::FObjectFinder<USoundBase> ReloadSoundAsset(TEXT("/Game/Sounds/Weapons/S_AK47_Reload"));
    if (ReloadSoundAsset.Succeeded())
    {
        ReloadSound = ReloadSoundAsset.Object;
    }

    static ConstructorHelpers::FObjectFinder<UAnimMontage> ReloadMontageAsset(TEXT("/Game/Animations/AM_AK47_Reload"));
    if (ReloadMontageAsset.Succeeded())
    {
        ReloadAnimation = ReloadMontageAsset.Object;
    }

    static ConstructorHelpers::FObjectFinder<UAnimMontage> FireMontageAsset(TEXT("/Game/Animations/AM_AK47_Fire"));
    if (FireMontageAsset.Succeeded())
    {
        FireAnimation = FireMontageAsset.Object;
    }
}


void AAK47::BeginPlay()
{
    Super::BeginPlay();
    
    // Создаем и устанавливаем поведение стрельбы
    UBulletFireBehavior* FireBehaviorObj = NewObject<UBulletFireBehavior>();
    if (FireBehaviorObj)
    {
        UE_LOG(LogTemp, Warning, TEXT("Created FireBehavior in BeginPlay"));
        SetFireBehavior(FireBehaviorObj);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create FireBehavior in BeginPlay"));
    }

    // Инициализируем магазин
    MagazineState.CurrentAmmo = MagazineState.MaxAmmo;
    UE_LOG(LogTemp, Warning, TEXT("Initialized magazine: %d/%d"), 
           MagazineState.CurrentAmmo, MagazineState.MaxAmmo);
}











void AAK47::SetupWeaponCollision()
{
    // Отключаем коллизию для оружия
    if (WeaponModel)
    {
        WeaponModel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        WeaponModel->SetCollisionResponseToAllChannels(ECR_Ignore);
    }

    // Прикрепляем к персонажу если есть
    if (ACharacter* OwningCharacter = Cast<ACharacter>(GetOwner()))
    {
        FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
        AttachToComponent(OwningCharacter->GetMesh(), AttachRules, FName("WeaponSocket"));
    }
}




void AAK47::Fire()
{
    UE_LOG(LogTemp, Warning, TEXT("AK47 Fire called"));
    
    if (!CanFire())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot fire: CurrentAmmo = %d"), MagazineState.CurrentAmmo);
        return;
    }

    // Проверяем FireBehavior
    if (FireBehavior)
    {
        UE_LOG(LogTemp, Warning, TEXT("Calling FireBehavior->Fire"));
        FireBehavior->Fire(this);
        ConsumeAmmo();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("FireBehavior is null!"));
    }

    PlayFireEffects();
    ApplyRecoil();
}




void AAK47::PlayFireEffects()
{
    // Проигрываем анимацию стрельбы
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

    // Спавним эффект гильзы
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
}

void AAK47::ApplyRecoil()
{
    if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
    {
        float RecoilPitch = FMath::RandRange(1.0f, 2.0f) * RecoilStrength;
        float RecoilYaw = FMath::RandRange(-0.5f, 0.5f) * RecoilStrength;
        
        Character->AddControllerPitchInput(-RecoilPitch * 0.05f);
        Character->AddControllerYawInput(RecoilYaw * 0.05f);
    }
}



void AAK47::StartFiring()
{
    UE_LOG(LogTemp, Warning, TEXT("AK47 StartFiring called"));
    if (!bIsFiring)
    {
        UE_LOG(LogTemp, Warning, TEXT("Starting to fire (bIsFiring was false)"));
        bIsFiring = true;
        Fire();
        
        // Проверяем настройку таймера
        if (GetWorld())
        {
            UE_LOG(LogTemp, Warning, TEXT("Setting up fire timer with rate: %f"), FireRate);
            GetWorld()->GetTimerManager().SetTimer(
                AutoFireTimerHandle,
                this,
                &AAK47::Fire,
                FireRate,
                true
            );
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("World is null in StartFiring"));
        }
    }
}










void AAK47::StopFiring()
{
    if (bIsFiring)
    {
        Super::StopFiring();
    }
}



void AAK47::Reload()
{
    UE_LOG(LogTemp, Warning, TEXT("AK47 Reload called. Current ammo: %d/%d"), 
           MagazineState.CurrentAmmo, MagazineState.MaxAmmo);

    if (MagazineState.CurrentAmmo == MagazineState.MaxAmmo)
    {
        UE_LOG(LogTemp, Warning, TEXT("Magazine already full"));
        return;
    }

    StopFiring();
    PlayReloadEffects();

    // Запускаем таймер перезарядки
    FTimerHandle ReloadTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        ReloadTimerHandle,
        this,
        &AAK47::ReloadMagazine,
        ReloadAnimation ? ReloadAnimation->GetPlayLength() : 2.0f,
        false
    );

    UE_LOG(LogTemp, Warning, TEXT("Started reload timer"));
}





void AAK47::PlayReloadEffects()
{
    // Проигрываем анимацию перезарядки
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

    // Проигрываем звук перезарядки
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
    int32 OldAmmo = MagazineState.CurrentAmmo;
    Super::ReloadMagazine();
    UE_LOG(LogTemp, Warning, TEXT("Magazine reloaded: %d -> %d"), 
           OldAmmo, MagazineState.CurrentAmmo);
    OnReloadComplete.Broadcast();
}