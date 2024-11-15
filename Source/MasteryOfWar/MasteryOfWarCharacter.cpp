#include "MasteryOfWarCharacter.h"
#include "MasteryOfWarGameMode.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "WeaponFactory.h"
#include "GameModeConfig.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AMasteryOfWarCharacter::AMasteryOfWarCharacter()
{
    // Set size for collision capsule
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

    // Use controller rotation for first person camera
    bUseControllerRotationPitch = true;
    bUseControllerRotationYaw = true;      
    bUseControllerRotationRoll = false;

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = false;  
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); 
    GetCharacterMovement()->JumpZVelocity = 500.f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 500.f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
    GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

    // Create first person camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FollowCamera->SetupAttachment(GetCapsuleComponent());
    FollowCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
    FollowCamera->bUsePawnControlRotation = true;

    // Create arms for first person view
    FPSArms = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPSArms"));
    FPSArms->SetupAttachment(FollowCamera);
    FPSArms->SetOnlyOwnerSee(true);
    FPSArms->bCastDynamicShadow = false;
    FPSArms->CastShadow = false;

    // Create weapon mesh component for editor preview
    WeaponMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMeshPreview"));
    WeaponMeshComponent->SetupAttachment(FPSArms, FName("WeaponSocket"));
    WeaponMeshComponent->SetRelativeLocation(FVector(-20.0f, 5.0f, -10.0f));
    WeaponMeshComponent->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
    WeaponMeshComponent->SetRelativeScale3D(FVector(0.1f));
    WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}



void AMasteryOfWarCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Проверяем иерархию компонентов
    if (!FollowCamera)
    {
        UE_LOG(LogTemp, Error, TEXT("BeginPlay: FollowCamera is null!"));
        return;
    }

    if (!FPSArms)
    {
        UE_LOG(LogTemp, Error, TEXT("BeginPlay: FPSArms is null!"));
        return;
    }

    if (!WeaponMeshComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("BeginPlay: WeaponMeshComponent is null!"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Component hierarchy: FollowCamera -> FPSArms -> WeaponMeshComponent verified"));
    
    // is WeaponMeshComponent attached to FPSArms ? 
    if (WeaponMeshComponent->GetAttachParent() != FPSArms)
    {
        UE_LOG(LogTemp, Error, TEXT("WeaponMeshComponent is not attached to FPSArms!"));
    }

    // Check socket
    FName SocketName = WeaponMeshComponent->GetAttachSocketName();
    UE_LOG(LogTemp, Warning, TEXT("WeaponMeshComponent attached to socket: %s"), *SocketName.ToString());

    // Add Input Mapping Context
    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
            UE_LOG(LogTemp, Warning, TEXT("Input mapping context added"));
        }
    }

    // Setup perspective-specific visibility
    if (IsLocallyControlled())
    {
        GetMesh()->SetOwnerNoSee(true);
        FPSArms->SetOwnerNoSee(false);
        if (CurrentWeapon && CurrentWeapon->WeaponMesh)
        {
            CurrentWeapon->WeaponMesh->SetOnlyOwnerSee(true);
        }
        UE_LOG(LogTemp, Warning, TEXT("Local player visibility set"));
    }
    else
    {
        GetMesh()->SetOwnerNoSee(false);
        FPSArms->SetOwnerNoSee(true);
        if (CurrentWeapon && CurrentWeapon->WeaponMesh)
        {
            CurrentWeapon->WeaponMesh->SetOwnerNoSee(true);
        }
        UE_LOG(LogTemp, Warning, TEXT("Remote player visibility set"));
    }

    // set up Blueprint weapon
    SetupExistingWeapon();
}


void AMasteryOfWarCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // check weapon state
    if (CurrentWeapon && !CurrentWeapon->IsValidLowLevel())
    {
        CurrentWeapon = nullptr;
        UE_LOG(LogTemp, Warning, TEXT("Invalid weapon reference detected and cleared"));
    }
}



void AMasteryOfWarCharacter::SetupExistingWeapon()
{
    TArray<AActor*> AttachedActors;
    GetAttachedActors(AttachedActors);
    
    for (AActor* Actor : AttachedActors)
    {
        if (AWeapon* WeaponActor = Cast<AWeapon>(Actor))
        {
            // delete old weapon if exits
            if (CurrentWeapon && CurrentWeapon != WeaponActor)
            {
                CurrentWeapon->Destroy();
                CurrentWeapon = nullptr;
            }
            
            CurrentWeapon = WeaponActor;
            
            // set up camera recoil
            if (UCameraRecoilComponent* RecoilComp = CurrentWeapon->CameraRecoilComponent)
            {
                RecoilComp->SetTargetCamera(FollowCamera);
                UE_LOG(LogTemp, Warning, TEXT("Camera recoil component set up for existing weapon"));
            }
            
            // weapon visibility
            if (UStaticMeshComponent* WeaponMesh = CurrentWeapon->WeaponMesh)
            {
                WeaponMesh->SetOnlyOwnerSee(IsLocallyControlled());
                WeaponMesh->SetOwnerNoSee(!IsLocallyControlled());
            }
            
            // set owner of weapon
            CurrentWeapon->SetOwner(this);
            
            UE_LOG(LogTemp, Warning, TEXT("Found and set up existing weapon from Blueprint"));
            break;
        }
    }
}



void AMasteryOfWarCharacter::InitializeForGameMode(const FGameModeConfig& ModeConfig)
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing character for game mode: %d"), (int32)ModeConfig.ModeType);

    CurrentModeConfig = ModeConfig;
    EquipWeaponForMode(ModeConfig.WeaponType);

    if (FPSArms && ModeConfig.AnimConfig.ArmsMesh)
    {
        FPSArms->SetSkeletalMesh(ModeConfig.AnimConfig.ArmsMesh);
        if (ModeConfig.AnimConfig.ArmsAnimClass)
        {
            FPSArms->SetAnimInstanceClass(ModeConfig.AnimConfig.ArmsAnimClass);
        }
    }

    if (GetMesh() && ModeConfig.AnimConfig.CharacterMesh)
    {
        GetMesh()->SetSkeletalMesh(ModeConfig.AnimConfig.CharacterMesh);
        if (ModeConfig.AnimConfig.AnimationClass)
        {
            GetMesh()->SetAnimInstanceClass(ModeConfig.AnimConfig.AnimationClass);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Character initialization complete"));
}



void AMasteryOfWarCharacter::EquipWeaponForMode(EWeaponType WeaponType)
{
    // check Blueprint weapon exists or not
    if (CurrentWeapon)
    {
        UE_LOG(LogTemp, Warning, TEXT("Weapon already exists from Blueprint, skipping weapon creation"));
        return;
    }

    if (!FPSArms)
    {
        UE_LOG(LogTemp, Error, TEXT("EquipWeaponForMode: FPSArms is null!"));
        return;
    }

    if (!WeaponMeshComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("EquipWeaponForMode: WeaponMeshComponent is null!"));
        return;
    }

    // create new weapon via weapon factory 
    if (UWorld* World = GetWorld())
    {
        FTransform SpawnTransform = WeaponMeshComponent->GetComponentTransform();
        
        if (AWeapon* NewWeapon = UWeaponFactory::CreateWeapon(World, WeaponType, SpawnTransform))
        {
            UE_LOG(LogTemp, Warning, TEXT("New weapon created"));
            CurrentWeapon = NewWeapon;
            
            // attach to FPSArms with rules
            FAttachmentTransformRules AttachRules(
                EAttachmentRule::SnapToTarget, // location
                EAttachmentRule::SnapToTarget, // rotation
                EAttachmentRule::KeepRelative, // scale
                true // Weld simulation
            );
            
            NewWeapon->AttachToComponent(FPSArms, AttachRules, WeaponMeshComponent->GetAttachSocketName());
            
            // set transform
            NewWeapon->SetActorRelativeLocation(WeaponMeshComponent->GetRelativeLocation());
            NewWeapon->SetActorRelativeRotation(WeaponMeshComponent->GetRelativeRotation());
            NewWeapon->SetActorRelativeScale3D(WeaponMeshComponent->GetRelativeScale3D());

            // set visibility 
            WeaponMeshComponent->SetVisibility(false);
            if (UStaticMeshComponent* WeaponMesh = NewWeapon->WeaponMesh)
            {
                WeaponMesh->SetOnlyOwnerSee(IsLocallyControlled());
                WeaponMesh->SetOwnerNoSee(!IsLocallyControlled());
            }
            
            NewWeapon->SetOwner(this);
            
            if (UCameraRecoilComponent* RecoilComp = NewWeapon->CameraRecoilComponent)
            {
                RecoilComp->SetTargetCamera(FollowCamera);
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Weapon attached to FPSArms at WeaponMeshComponent position"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create weapon!"));
        }
    }
}



void AMasteryOfWarCharacter::ApplyAnimationConfig(const FCharacterAnimConfig& AnimConfig)
{
    if (FPSArms && AnimConfig.ArmsMesh)
    {
        FPSArms->SetSkeletalMesh(AnimConfig.ArmsMesh);
        if (AnimConfig.ArmsAnimClass)
        {
            FPSArms->SetAnimInstanceClass(AnimConfig.ArmsAnimClass);
        }
    }

    if (GetMesh() && AnimConfig.CharacterMesh)
    {
        GetMesh()->SetSkeletalMesh(AnimConfig.CharacterMesh);
        if (AnimConfig.AnimationClass)
        {
            GetMesh()->SetAnimInstanceClass(AnimConfig.AnimationClass);
        }
    }
}


void AMasteryOfWarCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Jumping
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

        // Moving
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMasteryOfWarCharacter::Move);

        // Looking
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMasteryOfWarCharacter::Look);

        // Shooting
        EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AMasteryOfWarCharacter::StartFire);
        EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AMasteryOfWarCharacter::StopFire);

        // Reloading
        EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &AMasteryOfWarCharacter::OnReload);

        // Debug
        if (PlayerInputComponent)
        {
            PlayerInputComponent->BindKey(EKeys::Y, IE_Pressed, this, &AMasteryOfWarCharacter::ToggleDebugLine);
        }
    }
}

void AMasteryOfWarCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void AMasteryOfWarCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        // Mouse sens
        const float InterpSpeed = 20.0f;
        const float DeltaTime = GetWorld()->GetDeltaSeconds();
        
        float NewYaw = FMath::FInterpTo(0.0f, LookAxisVector.X, DeltaTime, InterpSpeed);
        float NewPitch = FMath::FInterpTo(0.0f, LookAxisVector.Y, DeltaTime, InterpSpeed);

        AddControllerYawInput(NewYaw);
        AddControllerPitchInput(NewPitch);
    }
}

void AMasteryOfWarCharacter::StartFire()
{
    if (CurrentWeapon)
    {
        CurrentWeapon->StartFiring();
    }
}

void AMasteryOfWarCharacter::StopFire()
{
    if (CurrentWeapon)
    {
        CurrentWeapon->StopFiring();
    }
}

void AMasteryOfWarCharacter::OnReload()
{
    if (CurrentWeapon)
    {
        CurrentWeapon->Reload();
    }
}

void AMasteryOfWarCharacter::ToggleDebugLine()
{
    bShowDebugLine = !bShowDebugLine;
    UE_LOG(LogTemp, Warning, TEXT("Debug line toggled: %s"), bShowDebugLine ? TEXT("On") : TEXT("Off"));
}