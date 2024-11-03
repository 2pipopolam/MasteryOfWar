#include "MasteryOfWarCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
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
    FollowCamera->SetRelativeLocation(FVector(30.0f, 0.0f, 64.0f));
    FollowCamera->bUsePawnControlRotation = true;
}

void AMasteryOfWarCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
    
    
    if (!FireAction)
    {
        UE_LOG(LogTemp, Warning, TEXT("FireAction is not set in character blueprint!"));
    }
    

    // create and attach weapon
    if (UWorld* World = GetWorld())
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = this;

        AAK47* Weapon = World->SpawnActor<AAK47>(AAK47::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
        
        if (Weapon)
        {
            UE_LOG(LogTemp, Warning, TEXT("Weapon spawned successfully"));
            AttachWeapon(Weapon);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to spawn weapon!"));
        }
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


void AMasteryOfWarCharacter::AttachWeapon(AAK47* Weapon)
{
    if (!Weapon) 
    {
        UE_LOG(LogTemp, Error, TEXT("Attempt to attach null weapon!"));
        return;
    }

    CurrentWeapon = Weapon;
    
    // attach gun to socket
    FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
    Weapon->AttachToComponent(GetMesh(), AttachRules, FName("WeaponSocket"));
    
    UE_LOG(LogTemp, Warning, TEXT("Weapon attached successfully"));
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
        
        // draw rays
        PlayerInputComponent->BindKey(EKeys::Y, IE_Pressed, this, &AMasteryOfWarCharacter::ToggleDebugLine);
    }
}


void AMasteryOfWarCharacter::ToggleDebugLine()
{
    bShowDebugLine = !bShowDebugLine;
    UE_LOG(LogTemp, Warning, TEXT("Debug line %s"), bShowDebugLine ? TEXT("enabled") : TEXT("disabled"));
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
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}