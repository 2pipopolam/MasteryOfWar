#include "NetworkGameServer.h"
#include <iostream>
#include <json/json.h>
#include <boost/asio.hpp>
#include <chrono>
#include <thread>
#include <memory>
#include <string>
#include <random>

NetworkGameServer& NetworkGameServer::getInstance()
{
    static NetworkGameServer instance;
    return instance;
}

NetworkGameServer::NetworkGameServer() : running(true), dbPath("DB/game.db") {}

NetworkGameServer::~NetworkGameServer() 
{
    stop();
}



bool NetworkGameServer::initialize(uint16_t port, const std::string& dbPath)
{
    this->dbPath = dbPath;
    
    sqlite3* db;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK)
    {
        std::cerr << "Failed to open database at " << dbPath << ": " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return false;
    }
    sqlite3_close(db);
    
    try 
    {
        acceptor = std::make_unique<tcp::acceptor>(io_context, 
            tcp::endpoint(tcp::v4(), port));
        startAccept();
        
        serverThread = std::thread([this]() {
            io_context.run();
        });

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

        // Обработка сообщений
        if (messageType == "REQUEST_PLAYER_ID") {
            return;
        }
        else if (messageType == "REGISTER") {
            handleRegister(root, socket);
        }
        else if (messageType == "AUTHENTICATE") {
            handleAuthenticate(root, socket);
        }
        else if (messageType == "CREATE_SESSION") {
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
        else if (messageType == "JOIN_SESSION") {
            int32_t sessionId = root["sessionId"].asInt();
            int32_t playerId = root["playerId"].asInt();
            std::string password = root["password"].asString();
            
            std::cout << "Join session request - Session: " << sessionId 
                      << ", Player: " << playerId << std::endl;
            
            bool success = joinGameSession(sessionId, playerId, password);
            
            Json::Value response;
            response["type"] = "SESSION_JOINED";
            response["success"] = success;
            response["sessionId"] = sessionId;
            
            std::string responseStr = Json::FastWriter().write(response) + '\0';
            boost::asio::write(socket, boost::asio::buffer(responseStr));

            if (success) {
                auto session = activeSessions[sessionId];
                
                Json::Value stateMsg;
                stateMsg["type"] = "SESSION_STATE";
                stateMsg["sessionId"] = sessionId;
                stateMsg["state"] = session->getSessionState();
                
                std::string stateStr = Json::FastWriter().write(stateMsg) + '\0';
                boost::asio::write(socket, boost::asio::buffer(stateStr));

                Json::Value notification;
                notification["type"] = "PLAYER_JOINED";
                notification["playerId"] = playerId;
                notification["sessionId"] = sessionId;
                
                std::string notificationStr = Json::FastWriter().write(notification) + '\0';
                broadcastToSession(sessionId, notificationStr);
            }
        }
        else if (messageType == "PLAYER_STATE") {
            handlePlayerState(root, socket);
        }
        else if (messageType == "SHOT") {
            handleShot(root, socket);
        }
        else if (messageType == "HIT_CONFIRM") {
            handleHitConfirmation(root, socket);
        }
        else if (messageType == "GET_SESSIONS") {
            std::cout << "Handling GET_SESSIONS request" << std::endl;
            auto sessions = getActiveSessions();
            
            Json::Value response;
            response["type"] = "SESSIONS_LIST";
            Json::Value sessionsArray(Json::arrayValue);
            
            for (const auto& session : sessions) {
                Json::Value sessionObj;
                sessionObj["sessionId"] = session.sessionId;
                sessionObj["mapType"] = static_cast<int>(session.mapType);
                sessionObj["currentPlayers"] = session.currentPlayers;
                sessionObj["hasPassword"] = session.hasPassword;
                sessionObj["sessionName"] = "Session " + std::to_string(session.sessionId);
                sessionsArray.append(sessionObj);
            }
            
            response["sessions"] = sessionsArray;
            std::string responseStr = Json::FastWriter().write(response) + '\0';
            boost::asio::write(socket, boost::asio::buffer(responseStr));
        }



        else if (messageType == "REQUEST_SESSION_STATE") {
            int32_t sessionId = root["sessionId"].asInt();
            std::lock_guard<std::mutex> lock(sessionsMutex);
            auto it = activeSessions.find(sessionId);
            if (it != activeSessions.end()) {
                Json::Value response;
                response["type"] = "SESSION_STATE";
                response["sessionId"] = sessionId;
                response["state"] = it->second->getSessionState();
                std::string responseStr = Json::FastWriter().write(response) + '\0';
                boost::asio::write(socket, boost::asio::buffer(responseStr));
            }
        }



        else if (messageType == "SESSION_STATE") {
            int32_t sessionId = root["sessionId"].asInt();
            auto it = activeSessions.find(sessionId);
            if (it == activeSessions.end() || it->second->getPlayerCount() > 2) {
                return;
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
    {
        std::lock_guard<std::mutex> sessionsLock(sessionsMutex);
        for (auto& [sessionId, session] : activeSessions)
        {
            const auto& players = session->getPlayers();
            if (std::find(players.begin(), players.end(), playerId) != players.end())
            {
                Json::Value notification;
                notification["type"] = "PLAYER_DISCONNECTED";
                notification["playerId"] = playerId;
                notification["sessionId"] = sessionId;
                
                std::string message = Json::FastWriter().write(notification);
                broadcastToSession(sessionId, message);
                
                session->removePlayer(playerId);
                break;
            }
        }
    }

    {
        std::lock_guard<std::mutex> clientsLock(clientsMutex);
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
    
    // Добавляем небольшую задержку перед отправкой следующих сообщений
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
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



    if (it->second->getPlayerCount() >= 2)
    {
        return false;
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

        std::cout << "Broadcasting to session " << sessionId << " with " << players.size() << " players" << std::endl;


        for (int32_t playerId : players) 
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            broadcastToPlayer(playerId, message);

            std::cout << "Attempting to broadcast to player " << playerId << std::endl;
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



bool NetworkGameServer::authenticateUser(const std::string& nickname, const std::string& password)
{
    sqlite3* db;
    if (sqlite3_open("DB/game.db", &db) != SQLITE_OK)
    {
        std::cerr << "Failed to open database" << std::endl;
        return false;
    }

    const char* query = "SELECT user_id, password, salt FROM Users WHERE nickname = ?";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, nickname.c_str(), -1, SQLITE_STATIC);
        
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            //int userId = sqlite3_column_int(stmt, 0);
            std::string storedHash = (const char*)sqlite3_column_text(stmt, 1);
            std::string storedSalt = (const char*)sqlite3_column_text(stmt, 2);

            std::string hashedPassword = hashPassword(password, storedSalt);
            
            sqlite3_finalize(stmt);
            sqlite3_close(db);

            return storedHash == hashedPassword;
        }
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return false;
}


std::string NetworkGameServer::hashPassword(const std::string& password, const std::string& salt)
{


    //bcrypt, scrypt или Argon2.

    
    // TODO: Заменить на реальную хеш-функцию
    // Например, используя OpenSSL:
    // unsigned char hash[SHA256_DIGEST_LENGTH];
    // SHA256_CTX sha256;
    // SHA256_Init(&sha256);
    // SHA256_Update(&sha256, saltedPassword.c_str(), saltedPassword.length());
    // SHA256_Final(hash, &sha256);


    std::string saltedPassword = password + salt;
    return saltedPassword; 
}



std::string NetworkGameServer::generateRandomSalt(size_t length)
{
    const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*";
    
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<int> distribution(0, charset.size() - 1);
    
    std::string salt;
    salt.reserve(length);
    
    for (size_t i = 0; i < length; ++i)
    {
        salt += charset[distribution(generator)];
    }
    
    return salt;
}



void NetworkGameServer::handleRegister(const Json::Value& root, tcp::socket& socket)
{
    std::string nickname = root["nickname"].asString();
    std::string password = root["password"].asString();
    
    bool success = false;
    int32_t userId = -1;
    std::string errorMessage;
    
    sqlite3* db;
    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc != SQLITE_OK)
    {
        errorMessage = sqlite3_errmsg(db);
        std::cerr << "Failed to open database: " << errorMessage << std::endl;
        sqlite3_close(db);
    }
    else
    {
        sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);
        
        const char* checkQuery = "SELECT user_id FROM Users WHERE nickname = ?";
        sqlite3_stmt* checkStmt;
        
        if (sqlite3_prepare_v2(db, checkQuery, -1, &checkStmt, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_text(checkStmt, 1, nickname.c_str(), -1, SQLITE_STATIC);
            
            if (sqlite3_step(checkStmt) == SQLITE_DONE) // Пользователь не существует
            {
                sqlite3_finalize(checkStmt);
                
                std::string salt = generateRandomSalt();
                std::string hashedPassword = hashPassword(password, salt);
                
                const char* insertQuery = 
                    "INSERT INTO Users (nickname, password, salt, avatar_path, created_at) "
                    "VALUES (?, ?, ?, 'default_avatar', CURRENT_TIMESTAMP)";
                    
                sqlite3_stmt* insertStmt;
                if (sqlite3_prepare_v2(db, insertQuery, -1, &insertStmt, nullptr) == SQLITE_OK)
                {
                    sqlite3_bind_text(insertStmt, 1, nickname.c_str(), -1, SQLITE_STATIC);
                    sqlite3_bind_text(insertStmt, 2, hashedPassword.c_str(), -1, SQLITE_STATIC);
                    sqlite3_bind_text(insertStmt, 3, salt.c_str(), -1, SQLITE_STATIC);
                    
                    if (sqlite3_step(insertStmt) == SQLITE_DONE)
                    {
                        userId = sqlite3_last_insert_rowid(db);
                        success = true;
                        sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr);
                    }
                    else
                    {
                        errorMessage = sqlite3_errmsg(db);
                        sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
                    }
                    
                    sqlite3_finalize(insertStmt);
                }
                else
                {
                    errorMessage = sqlite3_errmsg(db);
                    sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
                }
            }
            else
            {
                errorMessage = "User already exists";
                sqlite3_finalize(checkStmt);
                sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            }
        }
        else
        {
            errorMessage = sqlite3_errmsg(db);
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
        }
        
        sqlite3_close(db);
    }
    
    std::cout << "Registration attempt for user " << nickname 
              << ": " << (success ? "success" : "failed - " + errorMessage) << std::endl;
    
    Json::Value response;
    response["type"] = "REGISTRATION_RESULT";
    response["success"] = success;
    response["userId"] = userId;
    if (!success) {
        response["error"] = errorMessage;
    }
    
    std::string responseStr = Json::FastWriter().write(response) + '\0';
    boost::asio::write(socket, boost::asio::buffer(responseStr));
}




void NetworkGameServer::handleAuthenticate(const Json::Value& root, tcp::socket& socket)
{
    std::string nickname = root["nickname"].asString();
    std::string password = root["password"].asString();
    
    bool success = false;
    int32_t userId = -1;
    std::string errorMessage;
    
    sqlite3* db;
    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc != SQLITE_OK)
    {
        errorMessage = sqlite3_errmsg(db);
        std::cerr << "Failed to open database: " << errorMessage << std::endl;
        sqlite3_close(db);
    }
    else
    {
        const char* query = "SELECT user_id, password, salt FROM Users WHERE nickname = ?";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt, 1, nickname.c_str(), -1, SQLITE_STATIC);
            
            if (sqlite3_step(stmt) == SQLITE_ROW)
            {
                userId = sqlite3_column_int(stmt, 0);
                std::string storedHash = (const char*)sqlite3_column_text(stmt, 1);
                std::string storedSalt = (const char*)sqlite3_column_text(stmt, 2);
                
                std::string hashedPassword = hashPassword(password, storedSalt);
                success = (storedHash == hashedPassword);
                
                if (!success) {
                    errorMessage = "Invalid password";
                }
                
                std::cout << "Authentication details for " << nickname << ":" << std::endl
                         << "Stored hash: " << storedHash << std::endl
                         << "Generated hash: " << hashedPassword << std::endl;
            }
            else
            {
                errorMessage = "User not found";
            }
            sqlite3_finalize(stmt);
        }
        else
        {
            errorMessage = sqlite3_errmsg(db);
        }
        
        sqlite3_close(db);
    }
    
    std::cout << "Authentication attempt for user " << nickname 
              << ": " << (success ? "success" : "failed - " + errorMessage) << std::endl;
    
    Json::Value response;
    response["type"] = "AUTHENTICATION_RESULT";
    response["success"] = success;
    response["userId"] = userId;
    if (!success) {
        response["error"] = errorMessage;
    }
    
    std::string responseStr = Json::FastWriter().write(response) + '\0';
    boost::asio::write(socket, boost::asio::buffer(responseStr));
}