#include "AuthWidget.h"
#include "MofWGameInstance.h"
#include "DatabaseManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

void UAuthWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    if (SignUpButton)
        SignUpButton->OnClicked.AddDynamic(this, &UAuthWidget::OnSignUpClicked);
    
    if (LoginButton)
        LoginButton->OnClicked.AddDynamic(this, &UAuthWidget::OnLoginClicked);
    
    if (ErrorText)
        ErrorText->SetVisibility(ESlateVisibility::Hidden);
}

void UAuthWidget::OnSignUpClicked()
{
    if (!NicknameInput || !PasswordInput) 
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("SignUp failed: Input fields are null"));
        return;
    }
    
    FString Nickname = NicknameInput->GetText().ToString();
    FString Password = PasswordInput->GetText().ToString();
    
    FString ErrorMessage;
    if (!ValidateInput(Nickname, Password, ErrorMessage))
    {
        ShowError(ErrorMessage);
        return;
    }
    
    int32 UserId = FDatabaseManager::Get().RegisterUser(Nickname, Password);
    if (UserId > 0)
    {
        UWorld* World = GetWorld();
        if (!World)
        {
            return;
        }

        auto GameInstance = Cast<UMasteryOfWarGameInstance>(World->GetGameInstance());
        if (GameInstance)
        {
            GameInstance->SetCurrentUserId(UserId);
            GoToMainMenu();
        }
    }
    else
    {
        ShowError(TEXT("Failed to create account. Nickname might be taken."));
    }
}

void UAuthWidget::OnLoginClicked()
{
    if (!NicknameInput || !PasswordInput) 
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Login failed: Input fields are null"));
        return;
    }
    
    FString Nickname = NicknameInput->GetText().ToString();
    FString Password = PasswordInput->GetText().ToString();
    
    FString ErrorMessage;
    if (!ValidateInput(Nickname, Password, ErrorMessage))
    {
        ShowError(ErrorMessage);
        return;
    }
    
    int32 UserId = FDatabaseManager::Get().AuthenticateUser(Nickname, Password);
    
    if (UserId > 0)
    {
        UWorld* World = GetWorld();
        if (!World)
        {
            return;
        }

        auto GameInstance = Cast<UMasteryOfWarGameInstance>(World->GetGameInstance());
        if (GameInstance)
        {
            GameInstance->SetCurrentUserId(UserId);
            GoToMainMenu();
        }
    }
    else
    {
        ShowError(TEXT("Invalid nickname or password"));
    }
}

bool UAuthWidget::ValidateInput(const FString& Nickname, const FString& Password, FString& ErrorMessage)
{
    if (Nickname.IsEmpty() || Password.IsEmpty())
    {
        ErrorMessage = TEXT("Nickname and Password cannot be empty");
        return false;
    }
    
    if (Password.Len() < 6)
    {
        ErrorMessage = TEXT("Password must be at least 6 characters long");
        return false;
    }
    
    if (Nickname.Len() < 3)
    {
        ErrorMessage = TEXT("Nickname must be at least 3 characters long");
        return false;
    }
    
    for (TCHAR Character : Nickname)
    {
        if (!FChar::IsAlnum(Character) && Character != '_')
        {
            ErrorMessage = TEXT("Nickname can only contain letters, numbers and underscores");
            return false;
        }
    }
    
    return true;
}

void UAuthWidget::ShowError(const FString& Message)
{
    if (ErrorText)
    {
        ErrorText->SetText(FText::FromString(Message));
        ErrorText->SetVisibility(ESlateVisibility::Visible);
    }
}

void UAuthWidget::ClearError()
{
    if (ErrorText)
    {
        ErrorText->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UAuthWidget::GoToMainMenu()
{
    const FString LevelName = TEXT("/Game/MofW/Maps/MainMenuMap");
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    UGameplayStatics::OpenLevel(World, FName(*LevelName));
}