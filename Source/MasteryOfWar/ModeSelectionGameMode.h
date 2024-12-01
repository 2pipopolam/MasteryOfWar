#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ModeSelectionGameMode.generated.h"

UCLASS()
class MASTERYOFWAR_API AModeSelectionGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AModeSelectionGameMode();
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Network")
	FString ServerIP = TEXT("127.0.0.1");

	UPROPERTY(EditDefaultsOnly, Category = "Network")
	int32 ServerPort = 7777;
};