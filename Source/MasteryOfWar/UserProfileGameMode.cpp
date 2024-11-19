#include "UserProfileGameMode.h"
#include "Engine/Engine.h"

AUserProfileGameMode::AUserProfileGameMode()
{
	static ConstructorHelpers::FClassFinder<UUserProfileWidget> WidgetClassFinder(TEXT("WidgetBlueprint'/Game/MofW/Blueprints/WBP_UserProfile.WBP_UserProfile_C'"));
	if (WidgetClassFinder.Succeeded())
	{
		UserProfileWidgetClass = WidgetClassFinder.Class;
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Found Widget Class"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to find Widget Class"));
	}
}

void AUserProfileGameMode::BeginPlay()
{
	Super::BeginPlay();

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("BeginPlay Started"));

	APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
	if (!PC)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to get PlayerController"));
		return;
	}

	if (!UserProfileWidgetClass)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("UserProfileWidgetClass is null"));
		return;
	}

	UUserProfileWidget* ProfileWidget = CreateWidget<UUserProfileWidget>(PC, UserProfileWidgetClass);
	if (ProfileWidget)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Widget Created"));
		ProfileWidget->AddToViewport();
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Widget Added to Viewport"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to Create Widget"));
	}
}