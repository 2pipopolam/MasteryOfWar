#pragma once

#include <memory>
#include <vector>
#include <map>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#include <array>
#include <json/json.h>
#include <boost/asio.hpp>
#include "sqlite3.h"
#include "GameSession.h"
#include "ServerNetworkStructs.h"

using boost::asio::ip::tcp;
using namespace std::chrono_literals;

class NetworkGameServer {
public:
    static NetworkGameServer& getInstance();
    
    void stop();
    
    // Session management
    int32_t createGameSession(int32_t hostId, EGameMapType mapType, const std::string& password);
    bool joinGameSession(int32_t sessionId, int32_t playerId, const std::string& password);
    std::vector<GameSessionInfo> getActiveSessions();
    
    // Broadcast methods
    void broadcastToSession(int32_t sessionId, const std::string& message);
    void broadcastToPlayer(int32_t playerId, const std::string& message);
    
    bool initialize(uint16_t port, const std::string& dbPath = "../DB/game.db");


private:
    NetworkGameServer();
    ~NetworkGameServer();
    
    void startAccept();
    void startRead(std::shared_ptr<tcp::socket> socket, 
                  std::shared_ptr<std::array<char, 4096>> buffer,
                  int32_t clientId);
    void handleClientMessage(const std::string& msg, tcp::socket& socket);
    void cleanupInactiveSessions();
    void startCleanupTimer();
    
    // Message handlers
    void handlePlayerState(const Json::Value& root, [[maybe_unused]] tcp::socket& socket);
    void handleShot(const Json::Value& root, [[maybe_unused]] tcp::socket& socket);
    void handleHitConfirmation(const Json::Value& root, [[maybe_unused]] tcp::socket& socket);
    void handleDisconnect(int32_t playerId);


    bool authenticateUser(const std::string& nickname, const std::string& password);
    std::string hashPassword(const std::string& password, const std::string& salt);

    boost::asio::io_context io_context;
    std::unique_ptr<tcp::acceptor> acceptor;
    std::thread serverThread;
    std::thread cleanupThread;
    bool running;
    
    std::mutex sessionsMutex;
    std::mutex clientsMutex;
    
    std::map<int32_t, std::shared_ptr<GameSession>> activeSessions;
    std::map<int32_t, std::shared_ptr<tcp::socket>> clientSockets;
    std::map<int32_t, std::chrono::steady_clock::time_point> lastActivityTime;
    
    const std::chrono::seconds SESSION_TIMEOUT{300}; // 5 minutes
    const std::chrono::seconds CLEANUP_INTERVAL{60}; // 1 minute
    
    int32_t nextSessionId = 1;
    int32_t nextClientId = 1;

    std::string generateRandomSalt(size_t length = 16);

    std::string dbPath;

    void handleAuthenticate(const Json::Value& root, tcp::socket& socket);
    void handleRegister(const Json::Value& root, tcp::socket& socket);
};