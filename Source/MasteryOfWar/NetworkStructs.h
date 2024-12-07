#pragma once

#include "CoreMinimal.h"
#include "GameModeConfig.h"
#include "WeaponConfig.h"
#include "NetworkStructs.generated.h"

// Структура для информации о сессии
USTRUCT(BlueprintType)
struct FSessionInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Session")
    int32 SessionId;

    UPROPERTY(BlueprintReadWrite, Category = "Session")
    EGameMapType MapType;

    UPROPERTY(BlueprintReadWrite, Category = "Session")
    int32 CurrentPlayers;

    UPROPERTY(BlueprintReadWrite, Category = "Session")
    bool HasPassword;

    UPROPERTY(BlueprintReadWrite, Category = "Session")
    FString SessionName;

    FSessionInfo()
        : SessionId(-1)
        , MapType(EGameMapType::None)
        , CurrentPlayers(0)
        , HasPassword(false)
        , SessionName(TEXT(""))
    {
    }
};

// Структура состояния игрока для сети
USTRUCT(BlueprintType)
struct FNetworkPlayerState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    int32 PlayerId;

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    FVector Position;

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    FRotator Rotation;

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    bool bIsWalking;

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    bool bIsFiring;

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    bool bIsReloading;

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    int32 CurrentAmmo;

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    EWeaponType WeaponType;
};

// Структура информации о выстреле
USTRUCT(BlueprintType)
struct FNetworkShotInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    int32 ShooterId;

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    int32 BulletId;

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    FVector StartLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    FVector Direction;

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    float Speed;

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    int32 Damage;

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    float Spread;
};

// Структура информации о попадании
USTRUCT(BlueprintType)
struct FNetworkHitInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    int32 ShooterId;

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    int32 VictimId;

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    int32 BulletId;

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    FVector HitLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    FVector HitNormal;

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    float DamageTaken;
};

// Структура информации о броске гранаты
USTRUCT(BlueprintType)
struct FNetworkGrenadeInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    int32 ThrowerId;

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    FVector Location;

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    FRotator Rotation;

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    FVector Velocity;
};


USTRUCT(BlueprintType)
struct FNetworkSessionState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    int32 SessionId;

    UPROPERTY(BlueprintReadWrite, Category = "Network")
    TArray<FNetworkPlayerState> Players;


    FNetworkSessionState()
        : SessionId(-1)
    {
    }
};

// Delegate declarations for network events
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnNetworkError, int32, const FString&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionCreated, int32);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionsList, const TArray<FSessionInfo>&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionStateReceived, const FNetworkSessionState&);