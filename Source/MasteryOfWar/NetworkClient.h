#pragma once

#include "CoreMinimal.h"
#include "NetworkStructs.h"
#include "GameModeConfig.h"
#include "Sockets.h"
#include "SocketSubsystem.h"

// Network error codes
UENUM()
enum class ENetworkError : uint8
{
    None,
    ConnectionFailed,
    SessionCreationFailed,
    InvalidSessionData,
    ServerNotResponding,
    InvalidPassword,
    SessionFull
};

class NetworkClient
{
public:
    // Delegates for network events
    DECLARE_MULTICAST_DELEGATE_TwoParams(FOnNetworkError, int32 /*ErrorCode*/, const FString& /*ErrorMessage*/);
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionCreated, int32 /*SessionId*/);
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerStateReceived, const FNetworkPlayerState& /*State*/);
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnShotReceived, const FNetworkShotInfo& /*ShotInfo*/);
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnGrenadeThrowReceived, const FNetworkGrenadeInfo& /*GrenadeInfo*/);
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionsListReceived, const TArray<FSessionInfo>& /*Sessions*/);
    
    NetworkClient();
    ~NetworkClient();


    DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSessionJoined, bool /*Success*/, int32 /*SessionId*/);
    FOnSessionJoined OnSessionJoined;
    
    bool Connect(const FString& IPAddress, int32 Port);
    void Disconnect();
    bool IsConnected() const { return bConnected; }
    
    bool CreateSession(EGameMapType MapType, const FString& Password);
    bool JoinSession(int32 SessionId, const FString& Password);
    void RequestSessionsList();
    
    void SendPlayerState(const FNetworkPlayerState& State);
    void SendShot(const FNetworkShotInfo& ShotInfo);
    void SendGrenadeThrow(const FNetworkGrenadeInfo& GrenadeInfo);
    void SendHitConfirm(const FNetworkHitInfo& HitInfo);
    
    int32 GetPlayerId() const { return PlayerId; }
    
    // Network event delegates
    FOnNetworkError OnError;
    FOnSessionCreated OnSessionCreated;
    FOnPlayerStateReceived OnPlayerStateReceived;
    FOnShotReceived OnShotReceived;
    FOnGrenadeThrowReceived OnGrenadeThrowReceived;
    FOnSessionsListReceived OnSessionsListReceived;


    void SetPlayerId(int32 NewPlayerId);
    
private:
    int32 PlayerId;
    bool bConnected;
    TSharedPtr<FSocket> Socket;
    TSharedPtr<FInternetAddr> RemoteAddress;
    ENetworkError LastErrorCode;
    FString LastErrorMessage;

    int32 CurrentSessionId = -1;
    
    bool SendMessage(const FString& Message);
    void StartReceiveThread();
    void ReceiveLoop();
    void ProcessError(int32 ErrorCode, const FString& ErrorMessage);
    void HandleMessage(const FString& Message);
    void SetLastError(ENetworkError ErrorCode, const FString& Message);
    bool ValidateSessionParameters(EGameMapType MapType, const FString& Password);
};