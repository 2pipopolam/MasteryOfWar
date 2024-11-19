#include "UserProfileWidget.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"
#include "MofWGameInstance.h"
#include "DatabaseManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

void UUserProfileWidget::NativeConstruct()
{
    Super::NativeConstruct();

    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Widget NativeConstruct Started"));

    // Check all UI elements
    if (!NicknameInput) 
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("NicknameInput is null"));
        return;
    }

    if (!AvatarImage)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("AvatarImage is null"));
        return;
    }

    if (!ChangeAvatarButton)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("ChangeAvatarButton is null"));
        return;
    }

    if (!SaveNicknameButton)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("SaveNicknameButton is null"));
        return;
    }

    // handle events
    ChangeAvatarButton->OnClicked.AddDynamic(this, &UUserProfileWidget::OnChangeAvatarClicked);
    SaveNicknameButton->OnClicked.AddDynamic(this, &UUserProfileWidget::OnSaveNicknameClicked);
    
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Buttons bound successfully"));

    //  default avatar
    FString ImagePath = FPaths::ProjectContentDir() / TEXT("UserAvatars/default_avatar.png");
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Loading default avatar from: %s"), *ImagePath));

    UTexture2D* NewTexture = FImageUtils::ImportFileAsTexture2D(*ImagePath);
    if (NewTexture)
    {
        DefaultAvatar = NewTexture;
        AvatarImage->SetBrushFromTexture(DefaultAvatar);
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Default avatar loaded"));
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to load default avatar"));
    }

    LoadCurrentUserData();
}

void UUserProfileWidget::OnChangeAvatarClicked()
{
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Change Avatar Clicked"));

    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    if (!DesktopPlatform) 
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to get DesktopPlatform"));
        return;
    }

    TArray<FString> OutFilenames;
    const FString FileTypes = TEXT("Image Files (*.png;*.jpg;*.jpeg)|*.png;*.jpg;*.jpeg");

    const bool bOpened = DesktopPlatform->OpenFileDialog(
        nullptr,
        TEXT("Choose Avatar"),
        FPaths::ProjectDir(),
        TEXT(""),
        FileTypes,
        EFileDialogFlags::None,
        OutFilenames
    );

    if (!bOpened || OutFilenames.Num() == 0) 
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("No file selected or dialog cancelled"));
        return;
    }

    UMasteryOfWarGameInstance* GameInstance = Cast<UMasteryOfWarGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
    if (!GameInstance)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to get GameInstance"));
        return;
    }

    int32 CurrentUserId = GameInstance->GetCurrentUserId();
    if (CurrentUserId <= 0)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Invalid user ID"));
        return;
    }

    FString SelectedImagePath = OutFilenames[0];
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Selected image: %s"), *SelectedImagePath));

    FString AvatarDirectory = FPaths::ProjectDir() / TEXT("Content/UserAvatars/");
    FString NewFileName = FString::Printf(TEXT("avatar_%d_%s%s"),
        CurrentUserId,
        *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")),
        *FPaths::GetExtension(SelectedImagePath, true));

    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    
    // create directory if doesnt exist
    if (!PlatformFile.DirectoryExists(*AvatarDirectory))
    {
        if (!PlatformFile.CreateDirectoryTree(*AvatarDirectory))
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to create avatar directory"));
            return;
        }
    }

    // delete old avatar
    if (!CurrentAvatarPath.Equals(TEXT("default_avatar")))
    {
        FString OldAvatarPath = AvatarDirectory / CurrentAvatarPath;
        if (PlatformFile.FileExists(*OldAvatarPath))
        {
            if (!PlatformFile.DeleteFile(*OldAvatarPath))
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to delete old avatar"));
            }
        }
    }

    // copy new file
    FString NewAvatarPath = AvatarDirectory / NewFileName;
    if (PlatformFile.CopyFile(*NewAvatarPath, *SelectedImagePath))
    {
        if (FDatabaseManager::Get().UpdateUserAvatar(CurrentUserId, NewFileName))
        {
            UpdateAvatarImage(NewAvatarPath);
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Avatar updated successfully"));
        }
        else
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to update avatar in database"));
            // delete file if did not delete db
            PlatformFile.DeleteFile(*NewAvatarPath);
        }
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to copy avatar file"));
    }
}

void UUserProfileWidget::OnSaveNicknameClicked()
{
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Save Nickname Clicked"));

    if (!NicknameInput)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("NicknameInput is null"));
        return;
    }

    FString NewNickname = NicknameInput->GetText().ToString();
    if (NewNickname.IsEmpty())
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Nickname cannot be empty"));
        return;
    }

    UMasteryOfWarGameInstance* GameInstance = Cast<UMasteryOfWarGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
    if (!GameInstance)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to get GameInstance"));
        return;
    }

    int32 CurrentUserId = GameInstance->GetCurrentUserId();
    if (CurrentUserId <= 0)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Invalid user ID"));
        return;
    }

    if (FDatabaseManager::Get().UpdateUserNickname(CurrentUserId, NewNickname))
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Nickname updated successfully"));
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to update nickname. This nickname might be already taken"));
    }
}

void UUserProfileWidget::LoadCurrentUserData()
{
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Loading current user data"));

    UMasteryOfWarGameInstance* GameInstance = Cast<UMasteryOfWarGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
    if (!GameInstance)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to get GameInstance"));
        return;
    }

    int32 CurrentUserId = GameInstance->GetCurrentUserId();
    if (CurrentUserId <= 0)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Invalid user ID"));
        return;
    }

    auto UserData = FDatabaseManager::Get().GetUserData(CurrentUserId);
    
    if (NicknameInput)
    {
        NicknameInput->SetText(FText::FromString(UserData.Nickname));
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, 
            FString::Printf(TEXT("Loaded nickname: %s"), *UserData.Nickname));
    }

    if (!UserData.AvatarPath.Equals(TEXT("default_avatar")))
    {
        FString FullAvatarPath = FPaths::ProjectDir() / TEXT("Content/UserAvatars/") / UserData.AvatarPath;
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, 
            FString::Printf(TEXT("Loading avatar from: %s"), *FullAvatarPath));
        UpdateAvatarImage(FullAvatarPath);
    }

    CurrentAvatarPath = UserData.AvatarPath;
}

void UUserProfileWidget::UpdateAvatarImage(const FString& ImagePath)
{
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, 
        FString::Printf(TEXT("Updating avatar image from: %s"), *ImagePath));

    UTexture2D* NewTexture = FImageUtils::ImportFileAsTexture2D(*ImagePath);
    if (NewTexture && AvatarImage)
    {
        AvatarImage->SetBrushFromTexture(NewTexture);
        CurrentAvatarPath = FPaths::GetCleanFilename(ImagePath);
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Avatar image updated successfully"));
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to update avatar image"));
    }
}