#include "SessionBrowserWidget.h"
#include "MofWGameInstance.h"
#include "Kismet/GameplayStatics.h"


void USessionBrowserWidget::NativeConstruct()
{
    Super::NativeConstruct();

    GameInstance = Cast<UMasteryOfWarGameInstance>(GetGameInstance());
    if (!GameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get game instance"));
        return;
    }

    // Update current map type from game instance
    CurrentMapType = GameInstance->GetCurrentGameMode();
    UE_LOG(LogTemp, Warning, TEXT("SessionBrowser: Current map type set to: %d"), static_cast<int32>(CurrentMapType));

    // Initialize networking
    if (!GameInstance->IsConnectedToServer())
    {
        const FString ServerIP = TEXT("127.0.0.1");
        const int32 ServerPort = 7777;
       
        if (!GameInstance->InitializeNetworking(ServerIP, ServerPort))
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to initialize networking"));
            return;
        }
    }

    // Bind network events
    if (NetworkClient* Client = GameInstance->GetNetworkClient())
    {
        Client->OnSessionsListReceived.AddUObject(this, &USessionBrowserWidget::HandleSessionsList);
        UE_LOG(LogTemp, Log, TEXT("Network events bound successfully"));
    }

    // Bind UI events
    if (CreateSessionButton)
    {
        CreateSessionButton->OnClicked.AddDynamic(this, &USessionBrowserWidget::OnCreateSessionClicked);
        UE_LOG(LogTemp, Log, TEXT("Create session button bound"));
    }

    if (RefreshButton)
    {
        RefreshButton->OnClicked.AddDynamic(this, &USessionBrowserWidget::OnRefreshClicked);
    }

    if (BackButton)
    {
        BackButton->OnClicked.AddDynamic(this, &USessionBrowserWidget::OnBackClicked);
    }

    // Start periodic refresh
    GetWorld()->GetTimerManager().SetTimer(
        RefreshTimerHandle,
        this,
        &USessionBrowserWidget::RefreshSessionsList,
        2.0f, // Refresh every 2 seconds
        true  // Loop
    );

    // Initial refresh
    RefreshSessionsList();
    UE_LOG(LogTemp, Log, TEXT("SessionBrowserWidget initialized successfully"));
}



void USessionBrowserWidget::HandleSessionsList(const TArray<FSessionInfo>& Sessions)
{
    UE_LOG(LogTemp, Warning, TEXT("HandleSessionsList called with %d sessions"), Sessions.Num());
    
    // Clear existing entries
    if (!SessionsList)
    {
        UE_LOG(LogTemp, Error, TEXT("SessionsList is null"));
        return;
    }
    
    SessionsList->ClearChildren();
    
    // Add logging for current mode
    UE_LOG(LogTemp, Warning, TEXT("Current map type: %d"), static_cast<int32>(CurrentMapType));
    
    int32 addedSessions = 0;
    
    // Filter and add sessions
    for (const FSessionInfo& Session : Sessions)
    {
        UE_LOG(LogTemp, Warning, TEXT("Processing session %d with map type %d"), 
            Session.SessionId, 
            static_cast<int32>(Session.MapType));
            
        if (Session.MapType == CurrentMapType)
        {
            AddSessionEntry(Session);
            addedSessions++;
            UE_LOG(LogTemp, Warning, TEXT("Added session %d to UI"), Session.SessionId);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Skipped session %d - wrong map type"), Session.SessionId);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Added %d sessions to UI"), addedSessions);
}






void USessionBrowserWidget::SetupForGameMode(EGameMapType MapType)
{
    CurrentMapType = MapType;
    GameInstance = Cast<UMasteryOfWarGameInstance>(GetGameInstance());
    
    UE_LOG(LogTemp, Warning, TEXT("Setting up for map type: %d"), static_cast<int32>(MapType));
    
    if (!GameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get game instance"));
        return;
    }

    // Update sessions list for this mode
    RefreshSessionsList();
}

void USessionBrowserWidget::OnRefreshClicked()
{
    RefreshSessionsList();
}


void USessionBrowserWidget::OnCreateSessionClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Create Session Button Clicked"));

    if (!GameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("GameInstance is null in OnCreateSessionClicked"));
        return;
    }
    
    if (!GameInstance->IsConnectedToServer())
    {
        UE_LOG(LogTemp, Error, TEXT("Not connected to server in OnCreateSessionClicked"));
        return;
    }

    CurrentMapType = GameInstance->GetCurrentGameMode();
    UE_LOG(LogTemp, Warning, TEXT("Creating session for map type: %d"), static_cast<int32>(CurrentMapType));
    
    if (CurrentMapType == EGameMapType::None)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid map type for session creation"));
        return;
    }
    
    FString Password = PasswordInput ? PasswordInput->GetText().ToString() : FString();
    
    if (GameInstance->CreateGameSession(CurrentMapType, Password))
    {
        UE_LOG(LogTemp, Warning, TEXT("Session created successfully, opening map"));
        
        // Get corresponding map name for the mode
        FString MapName;
        switch (CurrentMapType)
        {
            case EGameMapType::Pistol_Map:
                MapName = TEXT("/Game/MofW/Maps/PistolMap");
                UE_LOG(LogTemp, Warning, TEXT("Opening Pistol Map"));
                break;
                
            case EGameMapType::Rifle_Map:
                MapName = TEXT("/Game/MofW/Maps/ThirdPersonMap");
                UE_LOG(LogTemp, Warning, TEXT("Opening Rifle Map"));
                break;
                
            case EGameMapType::Grenade_Map:
                MapName = TEXT("/Game/MofW/Maps/GrenadeMap");
                UE_LOG(LogTemp, Warning, TEXT("Opening Grenade Map"));
                break;
                
            default:
                UE_LOG(LogTemp, Error, TEXT("Invalid map type %d"), static_cast<int32>(CurrentMapType));
                return;
        }
        
        UGameplayStatics::OpenLevel(this, FName(*MapName));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create game session"));
    }
}


void USessionBrowserWidget::OnBackClicked()
{
    RemoveFromParent();
    UGameplayStatics::OpenLevel(this, TEXT("/Game/MofW/Maps/ModeSelectionMap"));
}







void USessionBrowserWidget::RefreshSessionsList()
{
    if (!GameInstance || !SessionsList) return;
    
    if (NetworkClient* Client = GameInstance->GetNetworkClient())
    {
        if (Client->GetPlayerId() <= 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("Waiting for valid player ID before refreshing sessions"));
            return;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Requesting sessions list"));
        Client->RequestSessionsList();
    }
}






void USessionBrowserWidget::AddSessionEntry(const FSessionInfo& SessionInfo)
{
    if (!SessionEntryWidgetClass || !SessionsList) return;
    
    USessionEntryWidget* EntryWidget = CreateWidget<USessionEntryWidget>(GetOwningPlayer(), SessionEntryWidgetClass);
    if (EntryWidget)
    {
        EntryWidget->SetSessionInfo(SessionInfo);
        SessionsList->AddChild(EntryWidget);
    }
}