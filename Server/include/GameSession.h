#pragma once
#include "ServerNetworkStructs.h"
#include <map>
#include <vector>
#include <string>
#include <chrono>
//#include <jsoncpp/json/json.h>


namespace Json {
    class Value;
}

class GameSession {
public:

   GameSession() : sessionId(-1) {
        lastActivityTime = std::chrono::steady_clock::now();
    }

    Json::Value getSessionState() const;


    void initialize(EGameMapType mapType, const std::string& password);
    bool addPlayer(int32_t playerId, const std::string& inputPassword);
    void removePlayer(int32_t playerId);
    
    // Combat functionality
    bool validateShot(const ShotInfo& shotInfo);
    void broadcastShot(const ShotInfo& shotInfo);
    bool validateGrenadeThrow(const GrenadeInfo& grenadeInfo);
    void broadcastGrenadeThrow(const GrenadeInfo& grenadeInfo);
    void broadcastPlayerState(const PlayerState& state);
    void broadcastHitConfirmation(const HitInfo& hitInfo);

    // Getters
    int32_t getPlayerCount() const { return static_cast<int32_t>(connectedPlayers.size()); }
    EGameMapType getMapType() const { return mapType; }
    const std::string& getPassword() const { return password; }
    const std::vector<int32_t>& getPlayers() const;

    // Session management
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
    EWeaponType GetDefaultWeaponTypeForMap(EGameMapType MapType) const;
};