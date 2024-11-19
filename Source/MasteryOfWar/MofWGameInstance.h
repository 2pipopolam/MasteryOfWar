#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MofWGameInstance.generated.h"

UCLASS()
class MASTERYOFWAR_API UMasteryOfWarGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UMasteryOfWarGameInstance(const FObjectInitializer& ObjectInitializer);
    
	void SetCurrentUserId(int32 UserId) { CurrentUserId = UserId; }
	int32 GetCurrentUserId() const { return CurrentUserId; }
	bool IsUserLoggedIn() const { return CurrentUserId > 0; }

private:
	int32 CurrentUserId = -1;
};