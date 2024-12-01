#pragma once
#include "ServerNetworkStructs.h"
#include <map>
#include <vector>
#include <string>
#include <chrono>

class GameSession {
public:
    GameSession() : sessionId(-1) {
        lastActivityTime = std::chrono::steady_clock::now();
    }

    void initialize(EGameMapType mapType, const std::string& password);
    bool addPlayer(int32_t playerId, const std::string& inputPassword);
    void removePlayer(int32_t playerId);
    bool validateShot(const ShotInfo& shotInfo);
    void broadcastPlayerState(const PlayerState& state);
    void broadcastShot(const ShotInfo& shotInfo);
    void broadcastHitConfirmation(const HitInfo& hitInfo);

    int32_t getPlayerCount() const { return static_cast<int32_t>(playerStates.size()); }
    EGameMapType getMapType() const { return mapType; }
    const std::string& getPassword() const { return password; }
    const std::vector<int32_t>& getPlayers() const;

    void updateLastActivity();
    bool isInactive(const std::chrono::seconds& timeout) const;
    void setSessionId(int32_t id);
    int32_t getSessionId() const;

private:
    int32_t sessionId;
    EGameMapType mapType;
    std::string password;
    std::map<int32_t, PlayerState> playerStates;
    std::vector<int32_t> connectedPlayers;
    std::chrono::steady_clock::time_point lastActivityTime;
};
