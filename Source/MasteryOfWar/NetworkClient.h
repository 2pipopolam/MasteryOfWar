#pragma once

#include "CoreMinimal.h"
#include "NetworkStructs.h"
#include "GameModeConfig.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "NetworkMapLoader.h"

class UWorld;
class UMofWGameInstance;

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
    DECLARE_MULTICAST_DELEGATE_TwoParams(FOnNetworkError, int32, const FString&);
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionCreated, int32);
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerStateReceived, const FNetworkPlayerState&);
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnShotReceived, const FNetworkShotInfo&);
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnGrenadeThrowReceived, const FNetworkGrenadeInfo&);
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionsListReceived, const TArray<FSessionInfo>&);
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerIdAssigned, int32);
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerJoined, int32);
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerLeft, int32);
    DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSessionJoined, bool, int32);
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionStateReceived, const FNetworkSessionState&);

    
    
    NetworkClient();
    ~NetworkClient();


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

    void RequestSessionState();


    int32 GetCurrentSessionId() const { return CurrentSessionId; }
    
    // Network event delegates
    FOnNetworkError OnError;
    FOnSessionCreated OnSessionCreated;
    FOnPlayerStateReceived OnPlayerStateReceived;
    FOnShotReceived OnShotReceived;
    FOnGrenadeThrowReceived OnGrenadeThrowReceived;
    FOnSessionsListReceived OnSessionsListReceived;

    FOnPlayerIdAssigned OnPlayerIdAssigned;

    FOnPlayerJoined OnPlayerJoined;
    FOnPlayerLeft OnPlayerLeft;

    
    FOnSessionStateReceived OnSessionStateReceived;

    
    void SetPlayerId(int32 NewPlayerId);
    void SetUserId(int32 NewUserId);

    void SetWorld(UWorld* InWorld) { CurrentWorld = InWorld; }

    void SetMapLoader(INetworkMapLoader* InMapLoader) { MapLoader = InMapLoader; }
    
private:
    int32 PlayerId;
    int32 UserId;  
    bool bConnected;
    TSharedPtr<FSocket> Socket;
    TSharedPtr<FInternetAddr> RemoteAddress;
    ENetworkError LastErrorCode;
    FString LastErrorMessage;

    int32 CurrentSessionId = -1;

    UWorld* CurrentWorld = nullptr;

    INetworkMapLoader* MapLoader = nullptr;
    
    bool SendMessage(const FString& Message);
    void StartReceiveThread();
    void ReceiveLoop();
    void ProcessError(int32 ErrorCode, const FString& ErrorMessage);
    void HandleMessage(const FString& Message);
    void SetLastError(ENetworkError ErrorCode, const FString& Message);
    bool ValidateSessionParameters(EGameMapType MapType, const FString& Password);
};