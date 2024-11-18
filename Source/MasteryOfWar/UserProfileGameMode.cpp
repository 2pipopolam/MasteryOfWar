#include "UserProfileGameMode.h"

AUserProfileGameMode::AUserProfileGameMode()
{
	static ConstructorHelpers::FClassFinder<UUserProfileWidget> WidgetClassFinder(TEXT("WidgetBlueprint'/Game/MofW/Blueprints/WBP_UserProfile.WBP_UserProfile_C'"));
	if (WidgetClassFinder.Succeeded())
	{
		UserProfileWidgetClass = WidgetClassFinder.Class;
	}
}

void AUserProfileGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		if (UserProfileWidgetClass)
		{
			UUserProfileWidget* ProfileWidget = CreateWidget<UUserProfileWidget>(PC, UserProfileWidgetClass);
			if (ProfileWidget)
			{
				ProfileWidget->AddToViewport();
			}
		}
	}
}