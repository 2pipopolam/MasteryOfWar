#include "NetworkGameServer.h"
#include <iostream>
#include <json/json.h>
#include <boost/asio.hpp>
#include <chrono>
#include <thread>
#include <memory>
#include <string>

NetworkGameServer& NetworkGameServer::getInstance()
{
    static NetworkGameServer instance;
    return instance;
}

NetworkGameServer::NetworkGameServer() : running(true) {}

NetworkGameServer::~NetworkGameServer() 
{
    stop();
}

bool NetworkGameServer::initialize(uint16_t port)
{
    try 
    {
        acceptor = std::make_unique<tcp::acceptor>(io_context, 
            tcp::endpoint(tcp::v4(), port));
        startAccept();
        
        // Start main IO context thread
        serverThread = std::thread([this]() {
            io_context.run();
        });

        // Start cleanup thread
        cleanupThread = std::thread([this]() {
            cleanupInactiveSessions();
        });
        
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Server initialization failed: " << e.what() << std::endl;
        return false;
    }
}


void NetworkGameServer::stop()
{
    std::cout << "Initiating server shutdown sequence..." << std::endl;
    
    // First, mark the server as not running to stop accept/cleanup loops
    running = false;
    
    try 
    {
        // Stop the IO context first to prevent new operations
        if (!io_context.stopped())
        {
            io_context.stop();
            std::cout << "IO context stopped successfully" << std::endl;
        }
        
        // Close the acceptor if it exists
        if (acceptor)
        {
            acceptor->close();
            std::cout << "Acceptor closed successfully" << std::endl;
        }
        
        // Wait for server thread to complete
        if (serverThread.joinable())
        {
            serverThread.join();
            std::cout << "Server thread joined successfully" << std::endl;
        }
        
        // Wait for cleanup thread to complete
        if (cleanupThread.joinable())
        {
            cleanupThread.join();
            std::cout << "Cleanup thread joined successfully" << std::endl;
        }

        // Close all client connections
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            for (auto& [clientId, socket] : clientSockets)
            {
                try 
                {
                    if (socket && socket->is_open())
                    {
                        socket->shutdown(boost::asio::socket_base::shutdown_both);
                        socket->close();
                        std::cout << "Closed connection for client ID: " << clientId << std::endl;
                    }
                }
                catch (const boost::system::system_error& e)
                {
                    std::cerr << "Error closing socket for client " << clientId 
                             << ": " << e.what() << std::endl;
                }
            }
            clientSockets.clear();
            lastActivityTime.clear();
        }

        // Clean up active sessions
        {
            std::lock_guard<std::mutex> lock(sessionsMutex);
            activeSessions.clear();
            std::cout << "All active sessions cleared" << std::endl;
        }
        
        std::cout << "Server shutdown completed successfully" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error during server shutdown: " << e.what() << std::endl;
    }
}



void NetworkGameServer::startAccept()
{
    auto socket = std::make_shared<tcp::socket>(io_context);
    acceptor->async_accept(*socket, [this, socket](const boost::system::error_code& error) {
        if (!error)
        {
            int32_t clientId = nextClientId++;

            Json::Value response;
            response["type"] = "PLAYER_ID_ASSIGNED";
            response["playerId"] = clientId;
            std::string message = Json::FastWriter().write(response);
            message += '\0';  // Добавляем нуль-терминатор
            boost::asio::write(*socket, boost::asio::buffer(message));
            
            std::cout << "Sent PLAYER_ID_ASSIGNED, client ID: " << clientId << std::endl;

            std::lock_guard<std::mutex> lock(clientsMutex);
            clientSockets[clientId] = socket;
            lastActivityTime[clientId] = std::chrono::steady_clock::now();
            
            auto buffer = std::make_shared<std::array<char, 4096>>();
            startRead(socket, buffer, clientId);
        }
        
        startAccept();
    });
}




void NetworkGameServer::startRead(std::shared_ptr<tcp::socket> socket, 
    std::shared_ptr<std::array<char, 4096>> buffer,
    int32_t clientId)
{
    socket->async_read_some(
        boost::asio::buffer(*buffer),
        [this, socket, buffer, clientId](const boost::system::error_code& error, std::size_t bytes_transferred) 
        {
            if (!error)
            {
                // Update last activity time
                {
                    std::lock_guard<std::mutex> lock(clientsMutex);
                    lastActivityTime[clientId] = std::chrono::steady_clock::now();
                }

                std::string msg(buffer->data(), bytes_transferred);
                handleClientMessage(msg, *socket);
                
                // Continue reading
                startRead(socket, buffer, clientId);
            }
            else
            {
                std::cerr << "Read error for client " << clientId << ": " << error.message() << std::endl;
                handleDisconnect(clientId);
            }
        });
}




void NetworkGameServer::handleClientMessage(const std::string& msg, tcp::socket& socket)
{
   try {
       Json::Value root;
       Json::Reader reader;
       if (!reader.parse(msg, root)) {
           std::cerr << "Failed to parse message: " << msg << std::endl;
           return;
       }

       std::string messageType = root["type"].asString();
       std::cout << "Received message type: " << messageType << std::endl;

       // Update last activity time
       if (root.isMember("playerId")) {
           int32_t playerId = root["playerId"].asInt();
           {
               std::lock_guard<std::mutex> lock(clientsMutex);
               lastActivityTime[playerId] = std::chrono::steady_clock::now();
           }
       }

       if (messageType == "REQUEST_PLAYER_ID") {
           return;
       }


       if (messageType == "CREATE_SESSION") 
       {
           int32_t hostId = root["hostId"].asInt();
           EGameMapType mapType = static_cast<EGameMapType>(root["mapType"].asInt());
           std::string password = root["password"].asString();
           
           std::cout << "Creating session request - HostID: " << hostId 
                     << ", MapType: " << static_cast<int>(mapType) << std::endl;
           
           int32_t sessionId = createGameSession(hostId, mapType, password);
           
           Json::Value response;
           response["type"] = "SESSION_CREATED";
           response["sessionId"] = sessionId;
           response["success"] = (sessionId != -1);
           
           std::string responseStr = Json::FastWriter().write(response);
           std::cout << "Sending response: " << responseStr << std::endl;
           
           boost::asio::write(socket, boost::asio::buffer(responseStr));
       }




        else if (messageType == "JOIN_SESSION") 
{
    int32_t sessionId = root["sessionId"].asInt();
    int32_t playerId = root["playerId"].asInt();
    std::string password = root["password"].asString();
    
    std::cout << "Join session request - Session: " << sessionId 
              << ", Player: " << playerId << std::endl;
    
    bool success = joinGameSession(sessionId, playerId, password);
    
    std::cout << "Join session result - Success: " << success << std::endl;

    // Отправляем ответ присоединившемуся клиенту
    Json::Value response;
    response["type"] = "SESSION_JOINED";
    response["success"] = success;
    response["sessionId"] = sessionId;
    
    std::string responseStr = Json::FastWriter().write(response);
    boost::asio::write(socket, boost::asio::buffer(responseStr));
    std::cout << "Sent join response: " << responseStr << std::endl;

    if (success)
    {
        Json::Value notification;
        notification["type"] = "PLAYER_JOINED";
        notification["playerId"] = playerId;
        notification["sessionId"] = sessionId;
        
        std::string notificationStr = Json::FastWriter().write(notification);
        broadcastToSession(sessionId, notificationStr);
        std::cout << "Broadcast join notification to session" << std::endl;
    }
}





       else if (messageType == "PLAYER_STATE") 
       {
           handlePlayerState(root, socket);
       }
       else if (messageType == "SHOT") 
       {
           handleShot(root, socket);
       }
       else if (messageType == "HIT_CONFIRM") 
       {
           handleHitConfirmation(root, socket);
       }



else if (messageType == "GET_SESSIONS") 
{
    std::cout << "Handling GET_SESSIONS request" << std::endl;
    auto sessions = getActiveSessions();
    
    Json::Value response;
    response["type"] = "SESSIONS_LIST";
    Json::Value sessionsArray(Json::arrayValue);
    
    std::cout << "Found " << sessions.size() << " available sessions" << std::endl;
    
    for (const auto& session : sessions) 
    {
        Json::Value sessionObj;
        sessionObj["sessionId"] = session.sessionId;
        sessionObj["mapType"] = static_cast<int>(session.mapType);
        sessionObj["currentPlayers"] = session.currentPlayers;
        sessionObj["hasPassword"] = session.hasPassword;
        sessionObj["sessionName"] = "Session " + std::to_string(session.sessionId);
        sessionsArray.append(sessionObj);
        
        std::cout << "Added session to response:" << std::endl
                  << "  ID: " << session.sessionId << std::endl
                  << "  MapType: " << static_cast<int>(session.mapType) << std::endl
                  << "  Players: " << session.currentPlayers << std::endl;
    }
    
    response["sessions"] = sessionsArray;
    std::string responseStr = Json::FastWriter().write(response) + '\0';  // Добавляем нуль-терминатор
    
    std::cout << "Sending sessions response: " << responseStr << std::endl;
    
    try {
        boost::asio::write(socket, boost::asio::buffer(responseStr));
        std::cout << "Sessions response sent successfully (" << responseStr.length() << " bytes)" << std::endl;
    }
    catch (const boost::system::system_error& e) {
        std::cerr << "Error sending sessions response: " << e.what() << std::endl;
    }
}



       else {
           std::cerr << "Unknown message type: " << messageType << std::endl;
       }
   }
   catch (const std::exception& e) {
       std::cerr << "Error handling client message: " << e.what() << std::endl;
       std::cerr << "Message content: " << msg << std::endl;
   }
}




void NetworkGameServer::handlePlayerState(const Json::Value& root, [[maybe_unused]] tcp::socket& socket)
{
    PlayerState state;
    state.playerId = root["playerId"].asInt();
    state.position = {
        root["position"]["x"].asFloat(),
        root["position"]["y"].asFloat(),
        root["position"]["z"].asFloat()
    };
    state.rotation = {
        root["rotation"]["x"].asFloat(),
        root["rotation"]["y"].asFloat(),
        root["rotation"]["z"].asFloat()
    };
    state.isCrouching = root["isCrouching"].asBool();
    state.isWalking = root["isWalking"].asBool();
    state.weapon.isFiring = root["weapon"]["isFiring"].asBool();
    state.weapon.isReloading = root["weapon"]["isReloading"].asBool();
    state.weapon.currentAmmo = root["weapon"]["currentAmmo"].asInt();

    int32_t sessionId = root["sessionId"].asInt();
    
    std::lock_guard<std::mutex> lock(sessionsMutex);
    auto it = activeSessions.find(sessionId);
    if (it != activeSessions.end()) {
        it->second->broadcastPlayerState(state);
    }
}

void NetworkGameServer::handleShot(const Json::Value& root, [[maybe_unused]] tcp::socket& socket)
{
    ShotInfo shotInfo;
    shotInfo.shooterId = root["shooterId"].asInt();
    shotInfo.startLocation = {
        root["startLocation"]["x"].asFloat(),
        root["startLocation"]["y"].asFloat(),
        root["startLocation"]["z"].asFloat()
    };
    shotInfo.direction = {
        root["direction"]["x"].asFloat(),
        root["direction"]["y"].asFloat(),
        root["direction"]["z"].asFloat()
    };
    shotInfo.speed = root["speed"].asFloat();
    shotInfo.damage = root["damage"].asInt();
    shotInfo.spread = root["spread"].asFloat();
    shotInfo.bulletId = root["bulletId"].asInt();

    int32_t sessionId = root["sessionId"].asInt();
    
    std::lock_guard<std::mutex> lock(sessionsMutex);
    auto it = activeSessions.find(sessionId);
    if (it != activeSessions.end() && it->second->validateShot(shotInfo)) {
        it->second->broadcastShot(shotInfo);
    }
}


/*
void NetworkGameServer::handleGrenadeThrow(const Json::Value& root, [[maybe_unused]] tcp::socket& socket)
{
    GrenadeInfo grenadeInfo;
    grenadeInfo.throwerId = root["throwerId"].asInt();
    grenadeInfo.location = {
        root["location"]["x"].asFloat(),
        root["location"]["y"].asFloat(),
        root["location"]["z"].asFloat()
    };
    grenadeInfo.velocity = {
        root["velocity"]["x"].asFloat(),
        root["velocity"]["y"].asFloat(),
        root["velocity"]["z"].asFloat()
    };

    int32_t sessionId = root["sessionId"].asInt();
    
    std::lock_guard<std::mutex> lock(sessionsMutex);
    auto it = activeSessions.find(sessionId);
    if (it != activeSessions.end() && it->second->validateGrenadeThrow(grenadeInfo)) {
        it->second->broadcastGrenadeThrow(grenadeInfo);
    }
}
*/



void NetworkGameServer::handleHitConfirmation(const Json::Value& root, [[maybe_unused]] tcp::socket& socket)
{
    HitInfo hitInfo;
    hitInfo.bulletId = root["bulletId"].asInt();
    hitInfo.hitLocation = {
        root["hitLocation"]["x"].asFloat(),
        root["hitLocation"]["y"].asFloat(),
        root["hitLocation"]["z"].asFloat()
    };
    hitInfo.hitNormal = {
        root["hitNormal"]["x"].asFloat(),
        root["hitNormal"]["y"].asFloat(),
        root["hitNormal"]["z"].asFloat()
    };
    hitInfo.damageTaken = root["damageTaken"].asFloat();

    int32_t sessionId = root["sessionId"].asInt();
    
    std::lock_guard<std::mutex> lock(sessionsMutex);
    auto it = activeSessions.find(sessionId);
    if (it != activeSessions.end()) {
        it->second->broadcastHitConfirmation(hitInfo);
    }
}

void NetworkGameServer::cleanupInactiveSessions()
{
    while (running) 
    {
        std::this_thread::sleep_for(CLEANUP_INTERVAL);
        
        auto now = std::chrono::steady_clock::now();
        
        {
            std::lock_guard<std::mutex> sessionsLock(sessionsMutex);
            std::lock_guard<std::mutex> clientsLock(clientsMutex);
            
            // Remove inactive clients
            for (auto it = lastActivityTime.begin(); it != lastActivityTime.end();) 
            {
                if (now - it->second > SESSION_TIMEOUT) 
                {
                    std::cout << "Removing inactive client: " << it->first << std::endl;
                    handleDisconnect(it->first);
                    it = lastActivityTime.erase(it);
                } 
                else 
                {
                    ++it;
                }
            }

            // Remove empty sessions
            for (auto it = activeSessions.begin(); it != activeSessions.end();)
            {
                if (it->second->getPlayerCount() == 0 || it->second->isInactive(SESSION_TIMEOUT))
                {
                    std::cout << "Removing inactive session: " << it->first << std::endl;
                    it = activeSessions.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }
    }
}

void NetworkGameServer::handleDisconnect(int32_t playerId)
{
    std::cout << "Client disconnected: " << playerId << std::endl;

    // Remove player from all sessions
    {
        std::lock_guard<std::mutex> lock(sessionsMutex);
        for (auto& [_, session] : activeSessions) {
            session->removePlayer(playerId);
        }

        // Remove empty sessions
        for (auto it = activeSessions.begin(); it != activeSessions.end();)
        {
            if (it->second->getPlayerCount() == 0)
            {
                it = activeSessions.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
    
    // Remove client socket
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clientSockets.erase(playerId);
        lastActivityTime.erase(playerId);
    }
}


int32_t NetworkGameServer::createGameSession(int32_t hostId, EGameMapType mapType, const std::string& password)
{
    std::lock_guard<std::mutex> lock(sessionsMutex);
    
    std::cout << "Creating session with map type: " << static_cast<int>(mapType) << std::endl;
    
    auto session = std::make_shared<GameSession>();
    session->initialize(mapType, password);
    session->setSessionId(nextSessionId);
    
    if (!session->addPlayer(hostId, password))
    {
        std::cerr << "Failed to add host to new session" << std::endl;
        return -1;
    }

    activeSessions[nextSessionId] = session;
    std::cout << "Created new session: " << nextSessionId << std::endl;
    return nextSessionId++;
}


bool NetworkGameServer::joinGameSession(int32_t sessionId, int32_t playerId, const std::string& password)
{
    std::lock_guard<std::mutex> lock(sessionsMutex);
    
    auto it = activeSessions.find(sessionId);
    if (it == activeSessions.end())
    {
        std::cerr << "Session not found: " << sessionId << std::endl;
        return false;
    }

    bool success = it->second->addPlayer(playerId, password);
    if (success)
    {
        std::cout << "Player " << playerId << " joined session " << sessionId << std::endl;
    }
    else
    {
        std::cerr << "Failed to add player " << playerId << " to session " << sessionId << std::endl;
    }

    return success;
}




std::vector<GameSessionInfo> NetworkGameServer::getActiveSessions()
{
    std::vector<GameSessionInfo> sessions;
    std::lock_guard<std::mutex> lock(sessionsMutex);
    
    std::cout << "\nGetting active sessions:" << std::endl;
    std::cout << "Total sessions in memory: " << activeSessions.size() << std::endl;
    
    for (const auto& [sessionId, session] : activeSessions)
    {
        std::cout << "\nAnalyzing session " << sessionId << ":" << std::endl;
        
        int32_t playerCount = session->getPlayerCount();
        EGameMapType mapType = session->getMapType();
        bool hasPassword = !session->getPassword().empty();
        
        std::cout << "  Players: " << playerCount << "/2" << std::endl;
        std::cout << "  MapType: " << static_cast<int>(mapType) << std::endl;
        std::cout << "  HasPassword: " << (hasPassword ? "yes" : "no") << std::endl;

        if (playerCount < 2)
        {
            GameSessionInfo info;
            info.sessionId = sessionId;
            info.mapType = mapType;
            info.currentPlayers = playerCount;
            info.hasPassword = hasPassword;
            info.sessionName = "Session " + std::to_string(sessionId);
            sessions.push_back(info);
            
            std::cout << "  Status: Added to response (has free slots)" << std::endl;
        }
        else 
        {
            std::cout << "  Status: Skipped (session is full)" << std::endl;
        }
    }
    
    std::cout << "\nReturning " << sessions.size() << " available sessions" << std::endl;
    return sessions;
}




void NetworkGameServer::broadcastToSession(int32_t sessionId, const std::string& message)
{
    std::lock_guard<std::mutex> lock(sessionsMutex);
    auto it = activeSessions.find(sessionId);
    if (it != activeSessions.end()) 
    {
        const auto& players = it->second->getPlayers();
        for (int32_t playerId : players) 
        {
            broadcastToPlayer(playerId, message);
        }
    }
    else
    {
        std::cerr << "Failed to broadcast: session " << sessionId << " not found" << std::endl;
    }
}



void NetworkGameServer::broadcastToPlayer(int32_t playerId, const std::string& message)
{
    std::lock_guard<std::mutex> lock(clientsMutex);
    auto it = clientSockets.find(playerId);
    if (it != clientSockets.end() && it->second) 
    {
        try 
        {
            // Добавляем нуль-терминатор к сообщению
            std::string messageWithNull = message + '\0';
            boost::asio::write(*(it->second), boost::asio::buffer(messageWithNull));
        }
        catch (const std::exception& e) 
        {
            std::cerr << "Error broadcasting to player " << playerId << ": " << e.what() << std::endl;
            handleDisconnect(playerId);
        }
    }
}