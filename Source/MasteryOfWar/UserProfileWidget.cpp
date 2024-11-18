#include "UserProfileWidget.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"

void UUserProfileWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ChangeAvatarButton)
	{
		ChangeAvatarButton->OnClicked.AddDynamic(this, &UUserProfileWidget::OnChangeAvatarClicked);
	}

	if (SaveNicknameButton)
	{
		SaveNicknameButton->OnClicked.AddDynamic(this, &UUserProfileWidget::OnSaveNicknameClicked);
	}

	// upload default avatar
	FString ImagePath = FPaths::ProjectContentDir() / TEXT("UserAvatars/default_avatar.png");
	UTexture2D* NewTexture = FImageUtils::ImportFileAsTexture2D(*ImagePath);
   
	if (NewTexture && AvatarImage)
	{
		DefaultAvatar = NewTexture;
		AvatarImage->SetBrushFromTexture(DefaultAvatar);
	}
}

void UUserProfileWidget::OnChangeAvatarClicked()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
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

		if (bOpened && OutFilenames.Num() > 0)
		{
			UpdateAvatarImage(OutFilenames[0]);
		}
	}
}

void UUserProfileWidget::OnSaveNicknameClicked()
{
	// TODO: impl  DatabaseManager
}

void UUserProfileWidget::UpdateAvatarImage(const FString& ImagePath)
{
	UTexture2D* NewTexture = FImageUtils::ImportFileAsTexture2D(ImagePath);
	if (NewTexture && AvatarImage)
	{
		AvatarImage->SetBrushFromTexture(NewTexture);
		CurrentAvatarPath = ImagePath;
	}
}