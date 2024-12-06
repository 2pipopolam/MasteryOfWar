#include "PlayerSpawnPoint.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Canvas.h"

APlayerSpawnPoint::APlayerSpawnPoint()
{
    PrimaryActorTick.bCanEverTick = false;
    TeamId = 0;
    bIsOccupied = false;

    // Создаем корневой компонент
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Создаем и настраиваем текстовый компонент
    TextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextComponent"));
    TextComponent->SetupAttachment(RootComponent);
    TextComponent->SetHorizontalAlignment(EHTA_Center);
    TextComponent->SetWorldSize(70.0f);  // Размер текста
    TextComponent->SetTextRenderColor(FColor::White);
}

void APlayerSpawnPoint::BeginPlay()
{
    Super::BeginPlay();
    
    #if WITH_EDITOR
        UpdateText();
    #else
        // В игре скрываем текст
        if (TextComponent)
        {
            TextComponent->SetVisibility(false);
        }
    #endif
}

void APlayerSpawnPoint::UpdateText()
{
    if (TextComponent)
    {
        FString TeamText = FString::Printf(TEXT("Spawn Point\nTeam %d"), TeamId);
        TextComponent->SetText(FText::FromString(TeamText));
        
        // Устанавливаем цвет в зависимости от команды
        FColor TeamColor = TeamId == 0 ? FColor::Blue : FColor::Red;
        TextComponent->SetTextRenderColor(TeamColor);
    }
}

#if WITH_EDITOR
void APlayerSpawnPoint::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    
    if (PropertyChangedEvent.Property)
    {
        const FName PropertyName = PropertyChangedEvent.Property->GetFName();
        if (PropertyName == GET_MEMBER_NAME_CHECKED(APlayerSpawnPoint, TeamId))
        {
            TeamId = FMath::Clamp(TeamId, 0, 1);
            UpdateText();
        }
    }
}

void APlayerSpawnPoint::EditorApplyRotation(const FRotator& DeltaRotation, bool bAltDown, bool bShiftDown, bool bCtrlDown)
{
    Super::EditorApplyRotation(DeltaRotation, bAltDown, bShiftDown, bCtrlDown);
    
    UpdateText();
}
#endif
