#include "ModeSelectionWidget.h"
#include "Kismet/GameplayStatics.h"
//#include "SessionBrowserWidget.h"
#include "MofWGameInstance.h"


void UModeSelectionWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    GameInstance = Cast<UMasteryOfWarGameInstance>(GetGameInstance());
    
    if (GameInstance)
    {
        if (!GameInstance->IsConnectedToServer())
        {
            const FString ServerIP = TEXT("127.0.0.1"); 
            const int32 ServerPort = 7777;  
            
            if (GameInstance->InitializeNetworking(ServerIP, ServerPort))
            {
                UE_LOG(LogTemp, Log, TEXT("Successfully connected to server"));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to connect to server"));
            }
        }
    }
    
    SetupButtonCallbacks();
}




void UModeSelectionWidget::SetupButtonCallbacks()
{
    if (Button_Pistol)
    {
        Button_Pistol->OnClicked.AddDynamic(this, &UModeSelectionWidget::OnPistolModeSelected);
    }

    if (Button_Rifle)
    {
        Button_Rifle->OnClicked.AddDynamic(this, &UModeSelectionWidget::OnRifleModeSelected);
    }

    if (Button_Grenade)
    {
        Button_Grenade->OnClicked.AddDynamic(this, &UModeSelectionWidget::OnGrenadeModeSelected);
    }

    if (Button_Back)
    {
        Button_Back->OnClicked.AddDynamic(this, &UModeSelectionWidget::OnBackSelected);
    }
}

/*
void UModeSelectionWidget::ShowSessionBrowser(EGameMapType MapType)
{
    if (!SessionBrowserWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("SessionBrowserWidgetClass is not set!"));
        return;
    }

    if (!GameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("GameInstance is not valid!"));
        return;
    }

    if (!GameInstance->IsConnectedToServer())
    {
        if (!GameInstance->InitializeNetworking())
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to connect to server"));
            return;
        }
    }

    if (CurrentSessionBrowser)
    {
        CurrentSessionBrowser->RemoveFromParent();
        CurrentSessionBrowser = nullptr;
    }

    CurrentSessionBrowser = CreateWidget<USessionBrowserWidget>(this, SessionBrowserWidgetClass);
    if (CurrentSessionBrowser)
    {
        CurrentSessionBrowser->SetupForGameMode(MapType);
        CurrentSessionBrowser->AddToViewport(1);
        UE_LOG(LogTemp, Log, TEXT("Session browser created for map type: %d"), static_cast<int32>(MapType));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create session browser widget"));
    }
}

void UModeSelectionWidget::OnPistolModeSelected()
{
    
    UE_LOG(LogTemp, Log, TEXT("Pistol Mode Selected"));
    UGameplayStatics::OpenLevel(this, TEXT("/Game/MofW/Maps/PistolMap"));
    //ShowSessionBrowser(EGameMapType::Pistol_Map);
}

void UModeSelectionWidget::OnRifleModeSelected()
{
    UE_LOG(LogTemp, Log, TEXT("Rifle Mode Selected"));
    UGameplayStatics::OpenLevel(this, TEXT("/Game/MofW/Maps/ThirdPersonMap"));
    //ShowSessionBrowser(EGameMapType::Rifle_Map);
}

void UModeSelectionWidget::OnGrenadeModeSelected()
{
    UE_LOG(LogTemp, Log, TEXT("Rifle Mode Selected"));
    UGameplayStatics::OpenLevel(this, TEXT("/Game/MofW/Maps/GrenadeMap"));
    //ShowSessionBrowser(EGameMapType::Rifle_Map);
}
*/


void UModeSelectionWidget::OnPistolModeSelected()
{
    UE_LOG(LogTemp, Log, TEXT("Pistol Mode Selected"));
    
    UGameplayStatics::OpenLevel(this, TEXT("/Game/MofW/Maps/SessionMap"));
    if (UMasteryOfWarGameInstance* GameInst = Cast<UMasteryOfWarGameInstance>(GetGameInstance()))
    {
        GameInst->SetCurrentGameMode(EGameMapType::Pistol_Map);
    }
}

void UModeSelectionWidget::OnRifleModeSelected()
{
    UE_LOG(LogTemp, Log, TEXT("Rifle Mode Selected"));
    
    UGameplayStatics::OpenLevel(this, TEXT("/Game/MofW/Maps/SessionMap"));
    if (UMasteryOfWarGameInstance* GameInst = Cast<UMasteryOfWarGameInstance>(GetGameInstance()))
    {
        GameInst->SetCurrentGameMode(EGameMapType::Rifle_Map);
    }
}


void UModeSelectionWidget::OnGrenadeModeSelected()
{
    UE_LOG(LogTemp, Log, TEXT("Rifle Mode Selected"));
    
    UGameplayStatics::OpenLevel(this, TEXT("/Game/MofW/Maps/SessionMap"));
    if (UMasteryOfWarGameInstance* GameInst = Cast<UMasteryOfWarGameInstance>(GetGameInstance()))
    {
        GameInst->SetCurrentGameMode(EGameMapType::Grenade_Map);
    }
}


void UModeSelectionWidget::OnBackSelected()
{
    UE_LOG(LogTemp, Log, TEXT("Back to main menu"));
    UGameplayStatics::OpenLevel(this, TEXT("/Game/MofW/Maps/MainMenuMap"));
}
