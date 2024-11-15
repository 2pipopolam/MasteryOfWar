#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AuthGameMode.generated.h"

struct sqlite3; 

UCLASS()
class MASTERYOFWAR_API AAuthGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAuthGameMode();
	virtual ~AAuthGameMode();

	UFUNCTION(BlueprintCallable, Category = "Auth")
	bool AuthSignUp(const FString& Nickname, const FString& Password);

	UFUNCTION(BlueprintCallable, Category = "Auth")
	bool AuthLogin(const FString& Nickname, const FString& Password);

	UFUNCTION(BlueprintCallable, Category = "Auth")
	void AuthLogout();

	UFUNCTION(BlueprintPure, Category = "Auth")
	bool IsUserLoggedIn() const { return bIsLoggedIn; }

	UFUNCTION(BlueprintPure, Category = "Auth")
	FString GetCurrentUser() const { return CurrentUser; }

private:
	sqlite3* Database;
	bool bIsLoggedIn;
	FString CurrentUser;
	int32 CurrentUserId;

	FString HashPassword(const FString& Password, const FString& Salt) const;
	FString GenerateSalt() const;
	bool VerifyPassword(const FString& InputPassword, const FString& StoredHash, const FString& StoredSalt) const;
	bool ExecuteQuery(const FString& Query, TArray<FString>& Results);
	bool ExecuteNonQuery(const FString& Query);
};