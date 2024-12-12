#include "GameSession.h"
#include "NetworkGameServer.h"
#include <json/json.h>


EWeaponType GameSession::GetDefaultWeaponTypeForMap(EGameMapType MapType) const
{
    switch(MapType)
    {
        case EGameMapType::Pistol_Map:
            return EWeaponType::DesertEagle;
        case EGameMapType::Rifle_Map:
            return EWeaponType::AK47;
        case EGameMapType::Grenade_Map:
            return EWeaponType::Grenade;
        default:
            return EWeaponType::AK47;
    }
}


Json::Value GameSession::getSessionState() const 
{
    Json::Value state;
    state["sessionId"] = sessionId;
    state["mapType"] = static_cast<int>(mapType);
    
    Json::Value players(Json::arrayValue);
    for (const auto& [playerId, playerState] : playerStates) {
        Json::Value player;
        player["playerId"] = playerId;
        
        Json::Value position;
        position["x"] = playerState.position.x;
        position["y"] = playerState.position.y;
        position["z"] = playerState.position.z;
        player["position"] = position;
        
        Json::Value rotation;
        rotation["x"] = playerState.rotation.x;
        rotation["y"] = playerState.rotation.y;
        rotation["z"] = playerState.rotation.z;
        player["rotation"] = rotation;
        
        player["isCrouching"] = playerState.isCrouching;
        player["isWalking"] = playerState.isWalking;
        
        Json::Value weapon;
        weapon["isFiring"] = playerState.weapon.isFiring;
        weapon["isReloading"] = playerState.weapon.isReloading;
        weapon["currentAmmo"] = playerState.weapon.currentAmmo;
        weapon["weaponType"] = static_cast<int>(playerState.weapon.weaponType);
        player["weapon"] = weapon;
        
        players.append(player);
    }
    state["players"] = players;
    return state;
}



void GameSession::broadcastPlayerState(const PlayerState& state)
{
    // Create the main JSON message
    Json::Value root;
    root["type"] = "PLAYER_STATE";
    root["playerId"] = state.playerId;
    root["sessionId"] = sessionId; 
    
    // Position data
    Json::Value position;
    position["x"] = state.position.x;
    position["y"] = state.position.y;
    position["z"] = state.position.z;
    root["position"] = position;
    
    // Rotation data
    Json::Value rotation;
    rotation["x"] = state.rotation.x;
    rotation["y"] = state.rotation.y;
    rotation["z"] = state.rotation.z;
    root["rotation"] = rotation;
    
    // Movement state
    root["isCrouching"] = state.isCrouching;
    root["isWalking"] = state.isWalking;
    
    // Weapon state
    Json::Value weapon;
    weapon["isFiring"] = state.weapon.isFiring;
    weapon["isReloading"] = state.weapon.isReloading;
    weapon["currentAmmo"] = state.weapon.currentAmmo;
    root["weapon"] = weapon;
    
    // Broadcast player state to all clients in session
    std::string message = Json::FastWriter().write(root);
    NetworkGameServer::getInstance().broadcastToSession(sessionId, message);
    
    // Update stored player state
    playerStates[state.playerId] = state;

    // Create and broadcast session state update
    Json::Value sessionState = getSessionState();
    Json::Value stateUpdate;
    stateUpdate["type"] = "SESSION_STATE";
    stateUpdate["sessionId"] = sessionId;
    stateUpdate["state"] = sessionState;
    
    std::string stateMessage = Json::FastWriter().write(stateUpdate);
    NetworkGameServer::getInstance().broadcastToSession(sessionId, stateMessage);
}



bool GameSession::validateShot(const ShotInfo& shotInfo)
{
    // Get shooter's state
    auto it = playerStates.find(shotInfo.shooterId);
    if (it == playerStates.end()) {
        return false;
    }
    
    const auto& shooterState = it->second;
    
    // Validate position
    float distanceThreshold = 100.0f; // Maximum allowed distance between reported and stored position
    Vector3 diff = {
        shotInfo.startLocation.x - shooterState.position.x,
        shotInfo.startLocation.y - shooterState.position.y,
        shotInfo.startLocation.z - shooterState.position.z
    };
    float distance = sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
    
    if (distance > distanceThreshold) {
        return false;
    }
    
    // Validate weapon state
    if (shooterState.weapon.isReloading || shooterState.weapon.currentAmmo <= 0) {
        return false;
    }
    
    // Validate direction vector
    float dirMagnitude = sqrt(
        shotInfo.direction.x * shotInfo.direction.x +
        shotInfo.direction.y * shotInfo.direction.y +
        shotInfo.direction.z * shotInfo.direction.z
    );
    
    if (abs(dirMagnitude - 1.0f) > 0.01f) {
        return false;
    }
    
    return true;
}

void GameSession::broadcastShot(const ShotInfo& shotInfo)
{
    Json::Value root;
    root["type"] = "SHOT";
    root["shooterId"] = shotInfo.shooterId;
    root["bulletId"] = shotInfo.bulletId;
    
    Json::Value startLocation;
    startLocation["x"] = shotInfo.startLocation.x;
    startLocation["y"] = shotInfo.startLocation.y;
    startLocation["z"] = shotInfo.startLocation.z;
    root["startLocation"] = startLocation;
    
    Json::Value direction;
    direction["x"] = shotInfo.direction.x;
    direction["y"] = shotInfo.direction.y;
    direction["z"] = shotInfo.direction.z;
    root["direction"] = direction;
    
    root["speed"] = shotInfo.speed;
    root["damage"] = shotInfo.damage;
    root["spread"] = shotInfo.spread;
    
    std::string message = Json::FastWriter().write(root);
    NetworkGameServer::getInstance().broadcastToSession(sessionId, message);
}

const std::vector<int32_t>& GameSession::getPlayers() const
{
    return connectedPlayers;
}




bool GameSession::validateGrenadeThrow(const GrenadeInfo& grenadeInfo)
{
    // Get thrower's state
    auto it = playerStates.find(grenadeInfo.throwerId);
    if (it == playerStates.end()) {
        return false;
    }
    
    const auto& throwerState = it->second;
    
    // Validate throw position
    float distanceThreshold = 100.0f;
    Vector3 diff = {
        grenadeInfo.location.x - throwerState.position.x,
        grenadeInfo.location.y - throwerState.position.y,
        grenadeInfo.location.z - throwerState.position.z
    };
    float distance = sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
    
    if (distance > distanceThreshold) {
        return false;
    }

    // Validate velocity
    float maxVelocity = 1000.0f;
    float velocityMagnitude = sqrt(
        grenadeInfo.velocity.x * grenadeInfo.velocity.x +
        grenadeInfo.velocity.y * grenadeInfo.velocity.y +
        grenadeInfo.velocity.z * grenadeInfo.velocity.z
    );
    
    if (velocityMagnitude > maxVelocity) {
        return false;
    }

    // Ensure only 2 players per session
    if (getPlayerCount() > 2) {
        return false;
    }
    
    return true;
}

void GameSession::broadcastGrenadeThrow(const GrenadeInfo& grenadeInfo)
{
    Json::Value root;
    root["type"] = "GRENADE_THROW";
    root["throwerId"] = grenadeInfo.throwerId;
    root["sessionId"] = sessionId;
    
    Json::Value location;
    location["x"] = grenadeInfo.location.x;
    location["y"] = grenadeInfo.location.y;
    location["z"] = grenadeInfo.location.z;
    root["location"] = location;
    
    Json::Value velocity;
    velocity["x"] = grenadeInfo.velocity.x;
    velocity["y"] = grenadeInfo.velocity.y;
    velocity["z"] = grenadeInfo.velocity.z;
    root["velocity"] = velocity;
    
    std::string message = Json::FastWriter().write(root);
    NetworkGameServer::getInstance().broadcastToSession(sessionId, message);
}





void GameSession::broadcastHitConfirmation(const HitInfo& hitInfo)
{
    Json::Value root;
    root["type"] = "HIT_CONFIRM";
    root["bulletId"] = hitInfo.bulletId;
    
    Json::Value hitLocation;
    hitLocation["x"] = hitInfo.hitLocation.x;
    hitLocation["y"] = hitInfo.hitLocation.y;
    hitLocation["z"] = hitInfo.hitLocation.z;
    root["hitLocation"] = hitLocation;
    
    Json::Value hitNormal;
    hitNormal["x"] = hitInfo.hitNormal.x;
    hitNormal["y"] = hitInfo.hitNormal.y;
    hitNormal["z"] = hitInfo.hitNormal.z;
    root["hitNormal"] = hitNormal;
    
    root["damageTaken"] = hitInfo.damageTaken;
    
    std::string message = Json::FastWriter().write(root);
    NetworkGameServer::getInstance().broadcastToSession(sessionId, message);
}

void GameSession::updateLastActivity()
{
    lastActivityTime = std::chrono::steady_clock::now();
}

bool GameSession::isInactive(const std::chrono::seconds& timeout) const
{
    auto now = std::chrono::steady_clock::now();
    return (now - lastActivityTime) > timeout;
}

void GameSession::setSessionId(int32_t id)
{
    sessionId = id;
}

int32_t GameSession::getSessionId() const
{
    return sessionId;
}


bool GameSession::addPlayer(int32_t playerId, const std::string& inputPassword)
{


    if (connectedPlayers.size() >= 2)
    {
        return false; 
    }



    // First validate the password if one is set
    if (!password.empty() && password != inputPassword) {
        return false;
    }
    
    // Check if player is already in the session
    if (std::find(connectedPlayers.begin(), connectedPlayers.end(), playerId) != connectedPlayers.end()) {
        return false;
    }
    
    // Initialize default player state
    PlayerState newState;
    newState.playerId = playerId;
    newState.position = Vector3(300, 0, 0);  // Default spawn position
    newState.rotation = Vector3(0, 0, 0);  // Default rotation
    newState.isCrouching = false;
    newState.isWalking = false;
    newState.weapon.isFiring = false;
    newState.weapon.isReloading = false;
    newState.weapon.currentAmmo = 30;      // Default ammo count

    newState.weapon.weaponType = GetDefaultWeaponTypeForMap(mapType); 

    
    // Add player to session
    playerStates[playerId] = newState;
    connectedPlayers.push_back(playerId);
    
    // Update last activity time
    updateLastActivity();
    
    return true;
}


void GameSession::removePlayer(int32_t playerId)
{
    auto it = playerStates.find(playerId);
    if (it != playerStates.end())
    {
        playerStates.erase(it);

        auto playerIt = std::find(connectedPlayers.begin(), connectedPlayers.end(), playerId);
        if (playerIt != connectedPlayers.end())
        {
            connectedPlayers.erase(playerIt);
        }
    }
}

void GameSession::initialize(EGameMapType mapType, const std::string& password)
{
    this->mapType = mapType;
    this->password = password;
    playerStates.clear();
    connectedPlayers.clear();
    lastActivityTime = std::chrono::steady_clock::now();
}