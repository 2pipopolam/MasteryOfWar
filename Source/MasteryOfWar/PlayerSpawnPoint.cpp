#include "PlayerSpawnPoint.h"
#include "Components/TextRenderComponent.h"
#include "Components/BillboardComponent.h"
#include "Engine/Canvas.h"

APlayerSpawnPoint::APlayerSpawnPoint()
{
    PrimaryActorTick.bCanEverTick = false;

    // Создаем компонент коллизии
    CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
    RootComponent = CollisionComponent;
    
    // Настраиваем коллизию
    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    CollisionComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    
    // Устанавливаем размер
    CollisionComponent->SetBoxExtent(FVector(50.0f));

    // Добавляем визуальное отображение в редакторе
#if WITH_EDITORONLY_DATA
    SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
    if (SpriteComponent)
    {
        SpriteComponent->SetupAttachment(RootComponent);
    }
#endif
}


FTransform APlayerSpawnPoint::GetSpawnTransform() const
{
    return GetActorTransform();
}




void APlayerSpawnPoint::BeginPlay()
{
    Super::BeginPlay();

    bIsOccupied = false;
    
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
