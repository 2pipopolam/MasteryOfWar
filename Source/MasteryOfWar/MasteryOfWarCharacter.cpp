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
#include "Kismet/GameplayStatics.h"
#include "GlobalArmsConfig.h"
#include "MofWGameInstance.h"
#include "NetworkClient.h"
#include "WeaponConfig.h"
#include "NetworkStructs.h"
#include "MofWGameInstance.h"


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
    GetCharacterMovement()->MaxWalkSpeed = DefaultSpeed;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
    GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

    // Create first person camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FollowCamera->SetupAttachment(GetCapsuleComponent());
    //FollowCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
    FollowCamera->SetRelativeLocation(FVector(0.0f, 0.0f, BaseEyeHeight));
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

    if (!FollowCamera)
    {
        //UE_LOG(LogTemp, Error, TEXT("BeginPlay: FollowCamera is null!"));
        return;
    }

    if (!FPSArms)
    {
        //UE_LOG(LogTemp, Error, TEXT("BeginPlay: FPSArms is null!"));
        return;
    }

    if (!WeaponMeshComponent)
    {
        //UE_LOG(LogTemp, Error, TEXT("BeginPlay: WeaponMeshComponent is null!"));
        return;
    }

    //UE_LOG(LogTemp, Warning, TEXT("Component hierarchy: FollowCamera -> FPSArms -> WeaponMeshComponent verified"));
    
    // Initialize movement settings
    GetCharacterMovement()->MaxWalkSpeed = DefaultSpeed;
    bIsWalking = false;

    /*
    if (WeaponMeshComponent->GetAttachParent() != FPSArms)
    {
        UE_LOG(LogTemp, Error, TEXT("WeaponMeshComponent is not attached to FPSArms!"));
    }
    */
    
    FName SocketName = WeaponMeshComponent->GetAttachSocketName();
    //UE_LOG(LogTemp, Warning, TEXT("WeaponMeshComponent attached to socket: %s"), *SocketName.ToString());

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
        //UE_LOG(LogTemp, Warning, TEXT("Local player visibility set"));
    }
    else
    {
        GetMesh()->SetOwnerNoSee(false);
        FPSArms->SetOwnerNoSee(true);
        if (CurrentWeapon && CurrentWeapon->WeaponMesh)
        {
            CurrentWeapon->WeaponMesh->SetOwnerNoSee(true);
        }
        //UE_LOG(LogTemp, Warning, TEXT("Remote player visibility set"));
    }

    // Set up Blueprint weapon
    SetupExistingWeapon();
    LoadAndApplyGlobalArmsPosition();

    GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
    //GetCharacterMovement()->CrouchedHalfHeight = 90.0f; //DEPRECATED

    //GetCharacterMovement()->SetCrouchedHalfHeight(90.0f);    

    GetCharacterMovement()->SetCrouchedHalfHeight(70.0f);
    GetCharacterMovement()->MaxWalkSpeedCrouched = 120.0f;



        if (UMasteryOfWarGameInstance* GameInstance = Cast<UMasteryOfWarGameInstance>(GetGameInstance()))
        {
            if (GameInstance->IsConnectedToServer())
            {
                GetWorld()->GetTimerManager().SetTimer(
                    NetworkUpdateTimer,
                    this,
                    &AMasteryOfWarCharacter::SendNetworkUpdate,
                    0.05f, // 20 раз в секунду
                    true
                );
            }
        }
}

void AMasteryOfWarCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Movement
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMasteryOfWarCharacter::Move);
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMasteryOfWarCharacter::Look);

        // Combat
        EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AMasteryOfWarCharacter::StartFire);
        EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AMasteryOfWarCharacter::StopFire);
        EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &AMasteryOfWarCharacter::OnReload);

        // Movement modifiers
        EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AMasteryOfWarCharacter::StartCrouch);
        EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AMasteryOfWarCharacter::StopCrouch);
        EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Started, this, &AMasteryOfWarCharacter::StartWalk);
        EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Completed, this, &AMasteryOfWarCharacter::StopWalk);

        //esc
        PlayerInputComponent->BindKey(EKeys::B, IE_Pressed, this, &AMasteryOfWarCharacter::OnBPressed);
        
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

void AMasteryOfWarCharacter::StartCrouch()
{
    Crouch();
}

void AMasteryOfWarCharacter::StopCrouch()
{
    UnCrouch();
}




void AMasteryOfWarCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
    
    if (FollowCamera)
    {
        FVector TargetLocation = FollowCamera->GetRelativeLocation();
        TargetLocation.Z = CrouchedEyeHeight - 30.0f;
        FollowCamera->SetRelativeLocation(TargetLocation);
    }   
}

void AMasteryOfWarCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
    
    if (FollowCamera)
    {
        FVector TargetLocation = FollowCamera->GetRelativeLocation();
        TargetLocation.Z = BaseEyeHeight;
        FollowCamera->SetRelativeLocation(TargetLocation);
    }
}




void AMasteryOfWarCharacter::StartWalk()
{
    bIsWalking = true;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    //UE_LOG(LogTemp, Verbose, TEXT("Walking started. Speed set to: %f"), WalkSpeed);
}

void AMasteryOfWarCharacter::StopWalk()
{
    bIsWalking = false;
    GetCharacterMovement()->MaxWalkSpeed = DefaultSpeed;
    //UE_LOG(LogTemp, Verbose, TEXT("Walking stopped. Speed reset to: %f"), DefaultSpeed);
}


void AMasteryOfWarCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CurrentWeapon && !CurrentWeapon->IsValidLowLevel())
    {
        CurrentWeapon = nullptr;
    }

    if (FollowCamera)
    {
        float TargetHeight = bIsCrouched ? (CrouchedEyeHeight - 30.0f) : BaseEyeHeight;
        FVector CurrentLocation = FollowCamera->GetRelativeLocation();
        
        float NewZ = FMath::FInterpTo(CurrentLocation.Z, TargetHeight, DeltaTime, 5.0f);
        
        FollowCamera->SetRelativeLocation(FVector(CurrentLocation.X, CurrentLocation.Y, NewZ));
    }

    // Сетевое обновление
    if (IsLocallyControlled())
    {
        static float TimeSinceLastUpdate = 0.0f;
        const float UpdateInterval = 0.05f; // 20 обновлений в секунду
        
        TimeSinceLastUpdate += DeltaTime;
        if (TimeSinceLastUpdate >= UpdateInterval)
        {
            SendNetworkUpdate();
            TimeSinceLastUpdate = 0.0f;
        }
    }
}


void AMasteryOfWarCharacter::SendNetworkUpdate()
{
    if (UMasteryOfWarGameInstance* GameInstance = Cast<UMasteryOfWarGameInstance>(GetGameInstance()))
    {
        if (NetworkClient* Client = GameInstance->GetNetworkClient())
        {
            FNetworkPlayerState State;
            State.PlayerId = GetPlayerId();
            State.Position = GetActorLocation();
            State.Rotation = GetActorRotation();
            State.bIsCrouching = bIsCrouched;
            State.bIsWalking = bIsWalking;
            
            if (CurrentWeapon)
            {
                State.bIsFiring = CurrentWeapon->IsFiring();
                State.bIsReloading = CurrentWeapon->IsReloading();
                State.CurrentAmmo = CurrentWeapon->GetCurrentAmmo();
                State.WeaponType = CurrentWeapon->GetWeaponType();
            }
            else
            {
                State.bIsFiring = false;
                State.bIsReloading = false;
                State.CurrentAmmo = 0;
                State.WeaponType = EWeaponType::None;
            }
            
            Client->SendPlayerState(State);
        }
    }
}



void AMasteryOfWarCharacter::EquipWeaponForMode(EWeaponType WeaponType)
{
    if (CurrentWeapon)
    {
        //UE_LOG(LogTemp, Warning, TEXT("Weapon already exists from Blueprint, skipping weapon creation"));
        return;
    }
    /*
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
    */

    if (UWorld* World = GetWorld())
    {
        FTransform SpawnTransform = WeaponMeshComponent->GetComponentTransform();
        
        if (AWeapon* NewWeapon = UWeaponFactory::CreateWeapon(World, WeaponType, SpawnTransform))
        {
            //UE_LOG(LogTemp, Warning, TEXT("New weapon created"));
            CurrentWeapon = NewWeapon;
            
            FAttachmentTransformRules AttachRules(
                EAttachmentRule::SnapToTarget,
                EAttachmentRule::SnapToTarget,
                EAttachmentRule::KeepRelative,
                true
            );
            
            NewWeapon->AttachToComponent(FPSArms, AttachRules, WeaponMeshComponent->GetAttachSocketName());
            
            NewWeapon->SetActorRelativeLocation(WeaponMeshComponent->GetRelativeLocation());
            NewWeapon->SetActorRelativeRotation(WeaponMeshComponent->GetRelativeRotation());
            NewWeapon->SetActorRelativeScale3D(WeaponMeshComponent->GetRelativeScale3D());

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
            
            //UE_LOG(LogTemp, Warning, TEXT("Weapon attached to FPSArms at WeaponMeshComponent position"));
        }
        
        //else
        //{
            //UE_LOG(LogTemp, Error, TEXT("Failed to create weapon!"));
        //}
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
            if (CurrentWeapon && CurrentWeapon != WeaponActor)
            {
                CurrentWeapon->Destroy();
                CurrentWeapon = nullptr;
            }
            
            CurrentWeapon = WeaponActor;
            
            if (UCameraRecoilComponent* RecoilComp = CurrentWeapon->CameraRecoilComponent)
            {
                RecoilComp->SetTargetCamera(FollowCamera);
                UE_LOG(LogTemp, Warning, TEXT("Camera recoil component set up for existing weapon"));
            }
            
            if (UStaticMeshComponent* WeaponMesh = CurrentWeapon->WeaponMesh)
            {
                WeaponMesh->SetOnlyOwnerSee(IsLocallyControlled());
                WeaponMesh->SetOwnerNoSee(!IsLocallyControlled());
            }
            
            CurrentWeapon->SetOwner(this);
            
            //UE_LOG(LogTemp, Warning, TEXT("Found and set up existing weapon from Blueprint"));
            break;
        }
    }
}

void AMasteryOfWarCharacter::InitializeForGameMode(const FGameModeConfig& ModeConfig)
{
    //UE_LOG(LogTemp, Warning, TEXT("Initializing character for game mode: %d"), (int32)ModeConfig.ModeType);

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

    //UE_LOG(LogTemp, Warning, TEXT("Character initialization complete"));
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

void AMasteryOfWarCharacter::SetArmsPosition(const FVector& NewPosition, const FRotator& NewRotation)
{
    if (FPSArms)
    {
        FPSArms->SetRelativeLocation(NewPosition);
        FPSArms->SetRelativeRotation(NewRotation);
        //UE_LOG(LogTemp, Warning, TEXT("Arms position updated - Position: %s, Rotation: %s"), 
            //*NewPosition.ToString(), *NewRotation.ToString());
    }
}

void AMasteryOfWarCharacter::SaveGlobalArmsPosition(const FVector& Position, const FRotator& Rotation)
{
    UGlobalArmsConfig* Config = Cast<UGlobalArmsConfig>(
        UGameplayStatics::CreateSaveGameObject(UGlobalArmsConfig::StaticClass()));
    
    Config->ArmsPosition.Position = Position;
    Config->ArmsPosition.Rotation = Rotation;
    
    if (UGameplayStatics::SaveGameToSlot(Config, "GlobalArmsConfig", 0))
    {
        //UE_LOG(LogTemp, Warning, TEXT("Global arms position saved"));
    }
}

void AMasteryOfWarCharacter::LoadAndApplyGlobalArmsPosition()
{
    FVector Position;
    FRotator Rotation;
    
    if (GetSavedArmsPosition(Position, Rotation))
    {
        SetArmsPosition(Position, Rotation);
        //UE_LOG(LogTemp, Warning, TEXT("Global arms position loaded and applied"));
    }
}

bool AMasteryOfWarCharacter::GetSavedArmsPosition(FVector& OutPosition, FRotator& OutRotation)
{
    if (UGlobalArmsConfig* Config = Cast<UGlobalArmsConfig>(
        UGameplayStatics::LoadGameFromSlot("GlobalArmsConfig", 0)))
    {
        OutPosition = Config->ArmsPosition.Position;
        OutRotation = Config->ArmsPosition.Rotation;
        return true;
    }
    return false;
}


void AMasteryOfWarCharacter::OnBPressed()
{
    if (UWorld* World = GetWorld())
    {
        UGameplayStatics::OpenLevel(World, FName("ModeSelectionMap"));
    }
}


void AMasteryOfWarCharacter::ToggleDebugLine()
{
    bShowDebugLine = !bShowDebugLine;
    //UE_LOG(LogTemp, Warning, TEXT("Debug line toggled: %s"), bShowDebugLine ? TEXT("On") : TEXT("Off"));
}


void AMasteryOfWarCharacter::UpdateFromNetworkState(const FNetworkPlayerState& State)
{
    SetActorLocation(State.Position);
    SetActorRotation(State.Rotation);
    
    if (State.bIsCrouching && !bIsCrouched)
        Crouch();
    else if (!State.bIsCrouching && bIsCrouched)
        UnCrouch();
        
    bIsWalking = State.bIsWalking;
    
    if (CurrentWeapon)
    {
        if (State.bIsFiring && !CurrentWeapon->IsFiring())
            CurrentWeapon->StartFiring();
        else if (!State.bIsFiring && CurrentWeapon->IsFiring())
            CurrentWeapon->StopFiring();
            
        if (State.bIsReloading && !CurrentWeapon->IsReloading())
            CurrentWeapon->Reload();
    }
}