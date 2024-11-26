// ArmsPositionConfigWidget.cpp
#include "ArmsPositionConfigWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"

void UArmsPositionConfigWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Initialize current weapon type as None
    CurrentWeaponType = EWeaponTypeEnum::None;

    // Initialize sliders and bind events
    InitializeSliders();
    
    // Bind slider events
    if (SliderPositionX)
    {
        SliderPositionX->OnValueChanged.AddDynamic(this, &UArmsPositionConfigWidget::OnPositionXChanged);
    }
    if (SliderPositionY)
    {
        SliderPositionY->OnValueChanged.AddDynamic(this, &UArmsPositionConfigWidget::OnPositionYChanged);
    }
    if (SliderPositionZ)
    {
        SliderPositionZ->OnValueChanged.AddDynamic(this, &UArmsPositionConfigWidget::OnPositionZChanged);
    }
    if (SliderRotationPitch)
    {
        SliderRotationPitch->OnValueChanged.AddDynamic(this, &UArmsPositionConfigWidget::OnRotationPitchChanged);
    }
    if (SliderRotationYaw)
    {
        SliderRotationYaw->OnValueChanged.AddDynamic(this, &UArmsPositionConfigWidget::OnRotationYawChanged);
    }
    if (SliderRotationRoll)
    {
        SliderRotationRoll->OnValueChanged.AddDynamic(this, &UArmsPositionConfigWidget::OnRotationRollChanged);
    }
    
    // Bind button events
    if (SaveButton)
    {
        SaveButton->OnClicked.AddDynamic(this, &UArmsPositionConfigWidget::OnSaveButtonClicked);
    }
    if (AK47Button)
    {
        AK47Button->OnClicked.AddDynamic(this, &UArmsPositionConfigWidget::OnAK47ButtonClicked);
    }
    if (DesertEagleButton)
    {
        DesertEagleButton->OnClicked.AddDynamic(this, &UArmsPositionConfigWidget::OnDesertEagleButtonClicked);
    }
    if (BackButton)
    {
        BackButton->OnClicked.AddDynamic(this, &UArmsPositionConfigWidget::OnBackClicked);
    }

    // Hide initial character mesh
    if (AMasteryOfWarCharacter* Character = Cast<AMasteryOfWarCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
    {
        HideCharacterMesh(Character);
    }
}

void UArmsPositionConfigWidget::InitializeSliders()
{
    // Set up position sliders range (-100 to 100)
    if (SliderPositionX)
    {
        SliderPositionX->SetMinValue(-100.0f);
        SliderPositionX->SetMaxValue(100.0f);
    }
    if (SliderPositionY)
    {
        SliderPositionY->SetMinValue(-100.0f);
        SliderPositionY->SetMaxValue(100.0f);
    }
    if (SliderPositionZ)
    {
        SliderPositionZ->SetMinValue(-100.0f);
        SliderPositionZ->SetMaxValue(100.0f);
    }

    // Set up rotation sliders range (-180 to 180)
    if (SliderRotationPitch)
    {
        SliderRotationPitch->SetMinValue(-180.0f);
        SliderRotationPitch->SetMaxValue(180.0f);
    }
    if (SliderRotationYaw)
    {
        SliderRotationYaw->SetMinValue(-180.0f);
        SliderRotationYaw->SetMaxValue(180.0f);
    }
    if (SliderRotationRoll)
    {
        SliderRotationRoll->SetMinValue(-180.0f);
        SliderRotationRoll->SetMaxValue(180.0f);
    }
}

void UArmsPositionConfigWidget::HideCharacterMesh(AMasteryOfWarCharacter* Character)
{
    if (Character)
    {
        // Hide third person mesh
        if (USkeletalMeshComponent* Mesh = Character->GetMesh())
        {
            Mesh->SetVisibility(false);
        }

        // Ensure FPS arms are visible
        if (USkeletalMeshComponent* FPSArms = Character->GetFPSArms())
        {
            FPSArms->SetVisibility(true);
        }
    }
}

void UArmsPositionConfigWidget::SpawnCharacter(const FString& CharacterPath, EWeaponTypeEnum NewWeaponType)
{
    // Skip if already showing this weapon type
    if (CurrentWeaponType == NewWeaponType)
    {
        return;
    }

    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        // Save current camera transform
        FVector Location = PC->GetPawn()->GetActorLocation();
        FRotator Rotation = PC->GetControlRotation();
        
        // Destroy current character
        PC->GetPawn()->Destroy();
        
        // Spawn parameters setup
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
        // Load and spawn new character
        if (UClass* CharacterClass = LoadClass<APawn>(nullptr, *CharacterPath))
        {
            if (APawn* NewPawn = GetWorld()->SpawnActor<APawn>(CharacterClass, Location, Rotation, SpawnParams))
            {
                PC->Possess(NewPawn);
                
                if (AMasteryOfWarCharacter* Character = Cast<AMasteryOfWarCharacter>(NewPawn))
                {
                    HideCharacterMesh(Character);

                    // Apply saved position if exists
                    FVector SavedPosition;
                    FRotator SavedRotation;
                    if (Character->GetSavedArmsPosition(SavedPosition, SavedRotation))
                    {
                        Character->SetArmsPosition(SavedPosition, SavedRotation);
                        
                        // Update current sliders to match saved position
                        CurrentPosition = SavedPosition;
                        CurrentRotation = SavedRotation;
                    }
                }
                
                // Update current weapon type
                CurrentWeaponType = NewWeaponType;
            }
        }
    }
}

void UArmsPositionConfigWidget::UpdateArmsPosition()
{
    if (AMasteryOfWarCharacter* Character = Cast<AMasteryOfWarCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
    {
        Character->SetArmsPosition(CurrentPosition, CurrentRotation);
    }
}

void UArmsPositionConfigWidget::OnSaveButtonClicked()
{
    AMasteryOfWarCharacter::SaveGlobalArmsPosition(CurrentPosition, CurrentRotation);
}

void UArmsPositionConfigWidget::OnAK47ButtonClicked()
{
    SpawnCharacter(TEXT("/Game/MofW/Blueprints/BP_MofWCharacter.BP_MofWCharacter_C"), EWeaponTypeEnum::AK47);
}

void UArmsPositionConfigWidget::OnDesertEagleButtonClicked()
{
    SpawnCharacter(TEXT("/Game/MofW/Blueprints/BP_MofWCharacterPistol.BP_MofWCharacterPistol_C"), EWeaponTypeEnum::DesertEagle);
}

void UArmsPositionConfigWidget::OnPositionXChanged(float Value)
{
    CurrentPosition.X = Value;
    UpdateArmsPosition();
}

void UArmsPositionConfigWidget::OnPositionYChanged(float Value)
{
    CurrentPosition.Y = Value;
    UpdateArmsPosition();
}

void UArmsPositionConfigWidget::OnPositionZChanged(float Value)
{
    CurrentPosition.Z = Value;
    UpdateArmsPosition();
}

void UArmsPositionConfigWidget::OnRotationPitchChanged(float Value)
{
    CurrentRotation.Pitch = Value;
    UpdateArmsPosition();
}

void UArmsPositionConfigWidget::OnRotationYawChanged(float Value)
{
    CurrentRotation.Yaw = Value;
    UpdateArmsPosition();
}

void UArmsPositionConfigWidget::OnRotationRollChanged(float Value)
{
    CurrentRotation.Roll = Value;
    UpdateArmsPosition();
}



void UArmsPositionConfigWidget::OnBackClicked()
{
    UGameplayStatics::OpenLevel(GetWorld(), FName("OptionsMenuMap"));
}