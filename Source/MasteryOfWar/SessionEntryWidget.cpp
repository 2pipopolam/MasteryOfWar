#include "SessionEntryWidget.h"
#include "MofWGameInstance.h"
#include "Kismet/GameplayStatics.h"

void USessionEntryWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    GameInstance = Cast<UMasteryOfWarGameInstance>(GetGameInstance());
    
    if (JoinButton)
    {
        JoinButton->OnClicked.AddDynamic(this, &USessionEntryWidget::OnJoinClicked);
    }
    
    // Subscribe to network events if we have a network client
    if (GameInstance && GameInstance->GetNetworkClient())
    {
        GameInstance->GetNetworkClient()->OnSessionJoined.AddUObject(this, &USessionEntryWidget::HandleSessionJoined);
    }
}

void USessionEntryWidget::SetSessionInfo(const FSessionInfo& Info)
{
    SessionInfo = Info;
    
    if (!SessionNameText || !PlayerCountText)
    {
        UE_LOG(LogTemp, Error, TEXT("Session Entry Widget UI components are null"));
        return;
    }
    
    FString SessionNameStr = FString::Printf(TEXT("Session %d"), Info.SessionId);
    SessionNameText->SetText(FText::FromString(SessionNameStr));
    PlayerCountText->SetText(FText::FromString(FString::Printf(TEXT("Players: %d/2"), Info.CurrentPlayers)));
    
    UE_LOG(LogTemp, Warning, TEXT("Session Entry Widget updated - ID: %d, Name: %s"), 
        Info.SessionId, *SessionNameStr);
}

void USessionEntryWidget::OnJoinClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Join button clicked for session %d"), SessionInfo.SessionId);
    
    if (!GameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("GameInstance is null in OnJoinClicked"));
        return;
    }
    
    int32 CurrentUserId = GameInstance->GetCurrentUserId();
    UE_LOG(LogTemp, Warning, TEXT("Attempting to join session %d as player %d"), 
        SessionInfo.SessionId, CurrentUserId);

    if (CurrentUserId <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid user ID for session join"));
        return;
    }
    
    // Show password dialog if needed
    FString Password = TEXT("");
    if (SessionInfo.HasPassword)
    {
        UE_LOG(LogTemp, Warning, TEXT("Session requires password"));
        // TODO: Implement password dialog
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Attempting to join session %d"), SessionInfo.SessionId);
    
    if (GameInstance->JoinGameSession(SessionInfo.SessionId, Password))
    {
        UE_LOG(LogTemp, Warning, TEXT("Join request sent for session %d"), SessionInfo.SessionId);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to send join request for session %d"), SessionInfo.SessionId);
        OnJoinSessionComplete.Broadcast(false);
    }
}

void USessionEntryWidget::HandleSessionJoined(bool Success, int32 SessionId)
{
    if (Success && SessionId == SessionInfo.SessionId)
    {
        UE_LOG(LogTemp, Warning, TEXT("Successfully joined session %d"), SessionId);
        
        // Open appropriate map based on session type
        FString MapName;
        switch (SessionInfo.MapType)
        {
            case EGameMapType::Pistol_Map:
                MapName = TEXT("/Game/MofW/Maps/PistolMap");
                break;
            case EGameMapType::Rifle_Map:
                MapName = TEXT("/Game/MofW/Maps/ThirdPersonMap");
                break;
            case EGameMapType::Grenade_Map:
                MapName = TEXT("/Game/MofW/Maps/GrenadeMap");
                break;
            default:
                UE_LOG(LogTemp, Error, TEXT("Invalid map type for session"));
                OnJoinSessionComplete.Broadcast(false);
                return;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Opening map: %s"), *MapName);
        UGameplayStatics::OpenLevel(this, FName(*MapName));
        OnJoinSessionComplete.Broadcast(true);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to join session %d"), SessionId);
        OnJoinSessionComplete.Broadcast(false);
    }
}