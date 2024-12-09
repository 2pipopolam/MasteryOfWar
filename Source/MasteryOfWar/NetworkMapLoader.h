#pragma once

#include "CoreMinimal.h"

class INetworkMapLoader
{
public:
	virtual ~INetworkMapLoader() = default;
	virtual void LoadNetworkMap(const FString& MapPath, int32 SessionId) = 0;
};