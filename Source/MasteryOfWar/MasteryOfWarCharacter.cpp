#include "MasteryOfWarCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "AK47.h"

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

    // Set movement properties
    GetCharacterMovement()->JumpZVelocity = 500.f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 500.f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
    GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

    // Create first person camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FollowCamera->SetupAttachment(GetCapsuleComponent());
    FollowCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f)); // Eye level
    FollowCamera->bUsePawnControlRotation = true;

    // Create arms for first person view
    FPSArms = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPSArms"));
    FPSArms->SetupAttachment(FollowCamera);
    FPSArms->SetOnlyOwnerSee(true);
    FPSArms->bCastDynamicShadow = false;
    FPSArms->CastShadow = false;

    // Create weapon mesh component for editor preview
    WeaponMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMeshPreview"));
    WeaponMeshComponent->SetupAttachment(FPSArms, WeaponSocketName);
    WeaponMeshComponent->SetRelativeLocation(FVector(-20.0f, 5.0f, -10.0f));
    WeaponMeshComponent->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
    WeaponMeshComponent->SetRelativeScale3D(FVector(0.1f));
    WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMasteryOfWarCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Add Input Mapping Context
    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }

    // Debug logging
    UE_LOG(LogTemp, Warning, TEXT("Character BeginPlay"));
    UE_LOG(LogTemp, Warning, TEXT("DefaultWeaponClass: %s"), 
           DefaultWeaponClass ? *DefaultWeaponClass->GetName() : TEXT("None"));
    UE_LOG(LogTemp, Warning, TEXT("WeaponSocketName: %s"), 
           *WeaponSocketName.ToString());

    // Hide editor preview weapon in game
    if (WeaponMeshComponent)
    {
        WeaponMeshComponent->SetVisibility(false);
    }

    // Setup perspective-specific visibility
    if (IsLocallyControlled())
    {
        GetMesh()->SetOwnerNoSee(true);
        FPSArms->SetOwnerNoSee(false);
    }
    else
    {
        GetMesh()->SetOwnerNoSee(false);
        FPSArms->SetOwnerNoSee(true);
    }

    // Spawn weapon if configured
    if (DefaultWeaponClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("Attempting to spawn weapon of class %s"), 
               *DefaultWeaponClass->GetName());

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = this;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        if (AWeapon* NewWeapon = GetWorld()->SpawnActor<AWeapon>(
            DefaultWeaponClass, 
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            SpawnParams))
        {
            CurrentWeapon = NewWeapon;
            UE_LOG(LogTemp, Warning, TEXT("Weapon spawned successfully"));
            
            if (FPSArms)
            {
                NewWeapon->AttachToComponent(
                    FPSArms,
                    FAttachmentTransformRules::SnapToTargetNotIncludingScale,
                    WeaponSocketName
                );
                
                // Match the preview weapon's transform
                NewWeapon->SetActorRelativeLocation(WeaponMeshComponent->GetRelativeLocation());
                NewWeapon->SetActorRelativeRotation(WeaponMeshComponent->GetRelativeRotation());
                NewWeapon->SetActorRelativeScale3D(WeaponMeshComponent->GetRelativeScale3D());
                
                UE_LOG(LogTemp, Warning, TEXT("Weapon attached successfully"));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("FPSArms is null"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to spawn weapon"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("DefaultWeaponClass not set in BP_MasteryOfWarCharacter"));
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

    UE_LOG(LogTemp, VeryVerbose, TEXT("Move Input: X=%f, Y=%f"), 
           MovementVector.X, MovementVector.Y);

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

    UE_LOG(LogTemp, VeryVerbose, TEXT("Look Input: X=%f, Y=%f"), 
           LookAxisVector.X, LookAxisVector.Y);

    if (Controller != nullptr)
    {
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}


void AMasteryOfWarCharacter::StartFire()
{
    UE_LOG(LogTemp, Warning, TEXT("StartFire called"));
    if (CurrentWeapon)
    {
        UE_LOG(LogTemp, Warning, TEXT("Has weapon, starting fire"));
        CurrentWeapon->StartFiring();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No weapon attached!"));
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
    UE_LOG(LogTemp, Warning, TEXT("OnReload called"));
    if (CurrentWeapon)
    {
        UE_LOG(LogTemp, Warning, TEXT("Weapon found, attempting reload"));
        CurrentWeapon->Reload();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No weapon found for reload"));
    }
}

void AMasteryOfWarCharacter::ToggleDebugLine()
{
    bShowDebugLine = !bShowDebugLine;
    UE_LOG(LogTemp, Warning, TEXT("Debug line %s"), 
           bShowDebugLine ? TEXT("enabled") : TEXT("disabled"));
}

void AMasteryOfWarCharacter::AttachWeapon(AAK47* Weapon)
{
    if (!Weapon)
    {
        UE_LOG(LogTemp, Error, TEXT("Attempt to attach null weapon!"));
        return;
    }

    CurrentWeapon = Weapon;
    
    if (FPSArms)
    {
        FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
        Weapon->AttachToComponent(FPSArms, AttachRules, WeaponSocketName);
        UE_LOG(LogTemp, Warning, TEXT("Weapon attached successfully to FPSArms"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("FPSArms component not found!"));
    }
}