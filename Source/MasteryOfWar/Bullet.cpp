/*
#include "Bullet.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"

ABullet::ABullet()
{
    PrimaryActorTick.bCanEverTick = true;

    // Создаем компоненты
    BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletMesh"));
    RootComponent = BulletMesh;

    // Настраиваем коллизию
    BulletMesh->SetCollisionProfileName(TEXT("Projectile"));
    BulletMesh->OnComponentHit.AddDynamic(this, &ABullet::OnBulletHit);

    // Создаем компонент движения снаряда
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = BulletMesh;
    ProjectileMovement->InitialSpeed = 500.0f;
    ProjectileMovement->MaxSpeed = 500.0f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->ProjectileGravityScale = 0.0f;

    // Время жизни пули
    InitialLifeSpan = 3.0f;
}

void ABullet::InitializeBullet(float Damage, float Speed, float MaxRange)
{
    WeaponDamage = Damage;
    ProjectileMovement->InitialSpeed = Speed;
    ProjectileMovement->MaxSpeed = Speed;
    MaxTravelDistance = MaxRange;
    StartLocation = GetActorLocation();
}

void ABullet::BeginPlay()
{
    Super::BeginPlay();
    StartLocation = GetActorLocation();
}

void ABullet::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Проверяем дистанцию полета
    float TravelDistance = FVector::Distance(StartLocation, GetActorLocation());
    if (TravelDistance > MaxTravelDistance)
    {
        Destroy();
    }
}


void ABullet::OnBulletHit(UPrimitiveComponent* HitComp, AActor* OtherActor, 
                         UPrimitiveComponent* OtherComp, FVector NormalImpulse, 
                         const FHitResult& Hit)
{
    if (OtherActor && OtherActor != GetOwner())
    {
        // Используем базовый FDamageEvent вместо FPointDamageEvent
        FDamageEvent DamageEvent;
        OtherActor->TakeDamage(WeaponDamage, DamageEvent, GetInstigatorController(), this);
    }

    Destroy();
}
*/


#include "Bullet.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/DamageType.h"  // Добавляем для FDamageEvent
#include "Engine/DamageEvents.h"       // Добавляем для FDamageEvent


ABullet::ABullet()
{
    PrimaryActorTick.bCanEverTick = true;

    // Создаем и настраиваем меш пули
    BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ARAmmo"));
    RootComponent = BulletMesh;
    
    // Загружаем базовый меш сферы для пули
    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
    if (MeshAsset.Succeeded())
    {
        BulletMesh->SetStaticMesh(MeshAsset.Object);
        BulletMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
    }

    // Настраиваем ProjectileMovement
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->SetUpdatedComponent(BulletMesh);
    ProjectileMovement->InitialSpeed = 8000.0f;
    ProjectileMovement->MaxSpeed = 8000.0f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->ProjectileGravityScale = 0.0f;
    ProjectileMovement->bShouldBounce = false;

    // Настраиваем коллизию
    BulletMesh->SetCollisionProfileName(TEXT("BlockAll"));
    BulletMesh->SetGenerateOverlapEvents(true);
    BulletMesh->SetNotifyRigidBodyCollision(true);
    BulletMesh->OnComponentHit.AddDynamic(this, &ABullet::OnBulletHit);
    
    InitialLifeSpan = 5.0f;
}



void ABullet::BeginPlay()
{
    Super::BeginPlay();
    StartLocation = GetActorLocation();
    
    // Рисуем отладочную сферу в точке спавна
    DrawDebugSphere(
        GetWorld(),
        GetActorLocation(),
        20.0f,
        12,
        FColor::Yellow,
        false,
        5.0f
    );

    UE_LOG(LogTemp, Warning, TEXT("Bullet spawned at location: %s with rotation: %s"), 
           *GetActorLocation().ToString(), *GetActorRotation().ToString());
    
    if (BulletMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("Bullet mesh is valid. Scale: %s"), 
               *BulletMesh->GetRelativeScale3D().ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Bullet mesh is null!"));
    }
}

void ABullet::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Рисуем след пули
    DrawDebugLine(
        GetWorld(),
        GetActorLocation() - GetActorForwardVector() * 50.0f,
        GetActorLocation(),
        FColor::Red,
        false,
        -1.0f,
        0,
        1.0f
    );

    float TravelDistance = FVector::Distance(StartLocation, GetActorLocation());
    if (TravelDistance > MaxTravelDistance)
    {
        Destroy();
    }
}

void ABullet::InitializeBullet(float Damage, float Speed, float MaxRange)
{
    WeaponDamage = Damage;
    ProjectileMovement->InitialSpeed = Speed;
    ProjectileMovement->MaxSpeed = Speed;
    MaxTravelDistance = MaxRange;
    StartLocation = GetActorLocation();
}


void ABullet::OnBulletHit(UPrimitiveComponent* HitComp, AActor* OtherActor, 
                         UPrimitiveComponent* OtherComp, FVector NormalImpulse, 
                         const FHitResult& Hit)
{
    if (OtherActor && OtherActor != GetOwner())
    {
        // Используем базовый класс урона
        const UDamageType* DamageType = UDamageType::StaticClass()->GetDefaultObject<UDamageType>();
        FDamageEvent DamageEvent(DamageType->GetClass());
        OtherActor->TakeDamage(WeaponDamage, DamageEvent, GetInstigatorController(), this);
        
        UE_LOG(LogTemp, Warning, TEXT("Bullet hit actor: %s with damage: %f"), 
               *OtherActor->GetName(), WeaponDamage);
        
        DrawDebugSphere(
            GetWorld(),
            Hit.ImpactPoint,
            10.0f,
            12,
            FColor::Red,
            false,
            2.0f
        );
    }

    Destroy();
}