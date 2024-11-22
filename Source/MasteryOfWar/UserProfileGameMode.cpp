#include "UserProfileGameMode.h"
#include "Engine/Engine.h"

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

	// Debug messages moved here where GEngine is guaranteed to be valid
	if (UserProfileWidgetClass)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Widget Class Found"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to find Widget Class"));
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
	if (!PC)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to get PlayerController"));
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