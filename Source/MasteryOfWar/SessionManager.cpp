#include "SessionManager.h"
#include "MofWGameInstance.h"
#include "NetworkClient.h"

ASessionManager::ASessionManager()
{
    PrimaryActorTick.bCanEverTick = false;
    CurrentSessionId = -1;
}

void ASessionManager::Initialize(UMasteryOfWarGameInstance* InGameInstance)
{
    GameInstance = InGameInstance;
    if (NetworkClient* Client = GameInstance->GetNetworkClient())
    {
        // Подписываемся на сетевые события
        Client->OnSessionStateReceived.AddUObject(this, &ASessionManager::HandleSessionState);
        Client->OnPlayerJoined.AddUObject(this, &ASessionManager::HandlePlayerJoined);
        Client->OnPlayerLeft.AddUObject(this, &ASessionManager::HandlePlayerLeft);
    }
}

bool ASessionManager::CreateSession(EGameMapType MapType, const FString& Password)
{
    if (!GameInstance || !GameInstance->GetNetworkClient()) return false;

    return GameInstance->GetNetworkClient()->CreateSession(MapType, Password);
}

bool ASessionManager::JoinSession(int32 SessionId, const FString& Password)
{
    if (!GameInstance || !GameInstance->GetNetworkClient()) return false;

    return GameInstance->GetNetworkClient()->JoinSession(SessionId, Password);
}

void ASessionManager::HandleSessionState(const FNetworkSessionState& State)
{
    CurrentSessionId = State.SessionId;
    ConnectedPlayers.Empty();
    
    for (const auto& PlayerState : State.Players)
    {
        ConnectedPlayers.AddUnique(PlayerState.PlayerId);
    }
    
    UpdateSessionState();
}

void ASessionManager::HandlePlayerJoined(int32 PlayerId)
{
    ConnectedPlayers.AddUnique(PlayerId);
    UpdateSessionState();
}

void ASessionManager::HandlePlayerLeft(int32 PlayerId)
{
    ConnectedPlayers.Remove(PlayerId);
    UpdateSessionState();
}

void ASessionManager::UpdateSessionState()
{
    // Обновляем состояние сессии
    if (GameInstance && GameInstance->GetNetworkClient())
    {
        GameInstance->GetNetworkClient()->RequestSessionState();
    }
}

void ASessionManager::LeaveSession()
{
    CleanupSession();
    if (GameInstance && GameInstance->GetNetworkClient())
    {
        GameInstance->GetNetworkClient()->Disconnect();
    }
}

void ASessionManager::CleanupSession()
{
    CurrentSessionId = -1;
    ConnectedPlayers.Empty();
}
