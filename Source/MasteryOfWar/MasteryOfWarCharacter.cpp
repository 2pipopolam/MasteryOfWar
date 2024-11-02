#include "MasteryOfWarCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "ShootingComponent.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);


AMasteryOfWarCharacter::AMasteryOfWarCharacter()
{
    // Set size for collision capsule
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

    bUseControllerRotationPitch = true;
    bUseControllerRotationYaw = true;      
    bUseControllerRotationRoll = false;

    if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
    {
        MovementComponent->GravityScale = 1.0f;
        
        MovementComponent->bOrientRotationToMovement = false;
        MovementComponent->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
        
        MovementComponent->JumpZVelocity = 500.f;
        MovementComponent->AirControl = 0.35f;
        
        MovementComponent->MaxWalkSpeed = 500.f;
        MovementComponent->MinAnalogWalkSpeed = 20.f;
        MovementComponent->BrakingDecelerationWalking = 2000.f;
        MovementComponent->BrakingDecelerationFalling = 1500.0f;
        
        
        MovementComponent->SetMovementMode(MOVE_Walking);
        MovementComponent->bConstrainToPlane = true;
        MovementComponent->bSnapToPlaneAtStart = true;
    }

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FollowCamera->SetupAttachment(GetCapsuleComponent());
    FollowCamera->SetRelativeLocation(FVector(30.0f, 0.0f, 64.0f));
    FollowCamera->bUsePawnControlRotation = true;

    // Create and attach shooting component
    ShootingComponent = CreateDefaultSubobject<UShootingComponent>(TEXT("ShootingComponent"));
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
        
        //Shooting
        EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AMasteryOfWarCharacter::OnFire);
        EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AMasteryOfWarCharacter::StopFire);
    }
    else
    {
        UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component!"), *GetNameSafe(this));
    }
}

void AMasteryOfWarCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        // find out which way is forward
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        // get forward vector
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    
        // get right vector 
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        // add movement 
        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void AMasteryOfWarCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void AMasteryOfWarCharacter::OnFire()
{
    if (ShootingComponent)
    {
        ShootingComponent->StartFiring();
    }
    else
    {
        UE_LOG(LogTemplateCharacter, Warning, TEXT("ShootingComponent is null in AMasteryOfWarCharacter::OnFire"));
    }
}

void AMasteryOfWarCharacter::StopFire()
{
    if (ShootingComponent)
    {
        ShootingComponent->StopFiring();
    }
}