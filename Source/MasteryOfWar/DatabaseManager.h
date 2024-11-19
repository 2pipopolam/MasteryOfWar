#pragma once

#include "CoreMinimal.h"
#include "sqlite3.h"

class MASTERYOFWAR_API FDatabaseManager
{
public:
	static FDatabaseManager& Get();
    
	struct FUserData
	{
		int32 UserId;
		FString Nickname;
		FString AvatarPath;
		FString Password;
		FString Salt;
	};

	// Auth methods
	int32 AuthenticateUser(const FString& Nickname, const FString& Password);
	int32 RegisterUser(const FString& Nickname, const FString& Password);
    
	// Profile methods
	bool UpdateUserAvatar(int32 UserId, const FString& NewAvatarPath);
	bool UpdateUserNickname(int32 UserId, const FString& NewNickname);
	FUserData GetUserData(int32 UserId);
    
private:
	FDatabaseManager();
	~FDatabaseManager();
    
	sqlite3* DB;
	bool Connect();
	void Close();
    
	FString HashPassword(const FString& Password, const FString& Salt);
	FString GenerateSalt();
    
	static FDatabaseManager* Instance;
};