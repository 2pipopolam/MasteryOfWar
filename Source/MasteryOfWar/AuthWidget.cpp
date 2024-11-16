#include "AuthWidget.h"
#include "AuthGameMode.h"
#include "Kismet/GameplayStatics.h"

void UAuthWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Bind button click events
    if (SignUpButton)
    {
        SignUpButton->OnClicked.AddDynamic(this, &UAuthWidget::OnSignUpClicked);
    }
    
    if (LoginButton)
    {
        LoginButton->OnClicked.AddDynamic(this, &UAuthWidget::OnLoginClicked);
    }
    
    // Clear error text initially
    if (ErrorText)
    {
        ErrorText->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UAuthWidget::OnSignUpClicked()
{
    if (!NicknameInput || !PasswordInput) return;
    
    FString Nickname = NicknameInput->GetText().ToString();
    FString Password = PasswordInput->GetText().ToString();
    
    // Validate input
    FString ErrorMessage;
    if (!ValidateInput(Nickname, Password, ErrorMessage))
    {
        ShowError(ErrorMessage);
        return;
    }
    
    // Get AuthGameMode
    if (AAuthGameMode* AuthGameMode = Cast<AAuthGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
    {
        // Try to sign up
        if (AuthGameMode->AuthSignUp(Nickname, Password))
        {
            GoToMainMenu();
        }
        else
        {
            ShowError(TEXT("Failed to create account. Nickname might be taken."));
        }
    }
}

void UAuthWidget::OnLoginClicked()
{
    if (!NicknameInput || !PasswordInput) return;
    
    FString Nickname = NicknameInput->GetText().ToString();
    FString Password = PasswordInput->GetText().ToString();
    
    // Validate input
    FString ErrorMessage;
    if (!ValidateInput(Nickname, Password, ErrorMessage))
    {
        ShowError(ErrorMessage);
        return;
    }
    
    // Get AuthGameMode
    if (AAuthGameMode* AuthGameMode = Cast<AAuthGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
    {
        // Try to login
        if (AuthGameMode->AuthLogin(Nickname, Password))
        {
            GoToMainMenu();
        }
        else
        {
            ShowError(TEXT("Invalid nickname or password"));
        }
    }
}

bool UAuthWidget::ValidateInput(const FString& Nickname, const FString& Password, FString& ErrorMessage)
{
    // Check for empty fields
    if (Nickname.IsEmpty() || Password.IsEmpty())
    {
        ErrorMessage = TEXT("Nickname and Password cannot be empty");
        return false;
    }
    
    // Check password length
    if (Password.Len() < 6)
    {
        ErrorMessage = TEXT("Password must be at least 6 characters long");
        return false;
    }
    
    // Check nickname length
    if (Nickname.Len() < 3)
    {
        ErrorMessage = TEXT("Nickname must be at least 3 characters long");
        return false;
    }
    
    // Check nickname characters (only letters, numbers and underscores allowed)
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
    UGameplayStatics::OpenLevel(GetWorld(), TEXT("MainMenuMap"));
}