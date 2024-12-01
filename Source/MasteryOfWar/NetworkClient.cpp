#include "NetworkClient.h"
#include "NetworkStructs.h"
#include "JsonObjectConverter.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "SocketTypes.h"
#include "GameModeConfig.h"
#include "IPAddress.h"
#include "Misc/StringBuilder.h"

NetworkClient::NetworkClient()
   : PlayerId(-1)
   , bConnected(false)
   , LastErrorCode(ENetworkError::None)
{
}

NetworkClient::~NetworkClient()
{
   Disconnect();
}

void NetworkClient::SetPlayerId(int32 NewPlayerId)
{
   PlayerId = NewPlayerId;
   UE_LOG(LogTemp, Warning, TEXT("NetworkClient PlayerId set to: %d"), PlayerId);
}

bool NetworkClient::Connect(const FString& IPAddress, int32 Port)
{
   if (bConnected)
   {
       return true;
   }

   RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
   bool bIsValid = false;
   RemoteAddress->SetIp(*IPAddress, bIsValid);
   if (!bIsValid)
   {
       SetLastError(ENetworkError::ConnectionFailed, TEXT("Invalid IP address"));
       return false;
   }
   RemoteAddress->SetPort(Port);

   FSocket* RawSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("GameClient"), false);
   
   if (!RawSocket)
   {
       SetLastError(ENetworkError::ConnectionFailed, TEXT("Failed to create socket"));
       return false;
   }

   int32 RecvSize = 64 * 1024;
   int32 SendSize = 64 * 1024;
   RawSocket->SetReceiveBufferSize(RecvSize, RecvSize);
   RawSocket->SetSendBufferSize(SendSize, SendSize);
   RawSocket->SetReuseAddr(true);

   Socket = MakeShareable(RawSocket);

   UE_LOG(LogTemp, Warning, TEXT("Attempting to connect to %s:%d"), *IPAddress, Port);

   if (!Socket->Connect(*RemoteAddress))
   {
       SetLastError(ENetworkError::ConnectionFailed, TEXT("Failed to connect to server"));
       return false;
   }

   bConnected = true;
   StartReceiveThread();

   UE_LOG(LogTemp, Warning, TEXT("Successfully connected to server at %s:%d"), *IPAddress, Port);
   return true;
}

void NetworkClient::Disconnect()
{
   if (Socket)
   {
       bConnected = false;
       Socket->Close();
       Socket = nullptr;
   }
   PlayerId = -1;
}

bool NetworkClient::CreateSession(EGameMapType MapType, const FString& Password)
{
   if (!IsConnected())
   {
       SetLastError(ENetworkError::ConnectionFailed, TEXT("Not connected to server"));
       return false;
   }
   
   UE_LOG(LogTemp, Warning, TEXT("Creating session - MapType: %d, PlayerId: %d"), 
       static_cast<int32>(MapType), PlayerId);
   
   TSharedPtr<FJsonObject> JsonObj = MakeShared<FJsonObject>();
   JsonObj->SetStringField(TEXT("type"), TEXT("CREATE_SESSION"));
   JsonObj->SetNumberField(TEXT("mapType"), static_cast<int32>(MapType));
   JsonObj->SetStringField(TEXT("password"), Password);
   JsonObj->SetNumberField(TEXT("hostId"), PlayerId);

   FString Message;
   TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Message);
   FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);

   return SendMessage(Message);
}

bool NetworkClient::JoinSession(int32 SessionId, const FString& Password)
{
   if (!IsConnected())
   {
       SetLastError(ENetworkError::ConnectionFailed, TEXT("Not connected to server"));
       return false;
   }

   UE_LOG(LogTemp, Warning, TEXT("Sending join request - Session: %d, Player: %d"), 
       SessionId, PlayerId);

   TSharedPtr<FJsonObject> JsonObj = MakeShared<FJsonObject>();
   JsonObj->SetStringField(TEXT("type"), TEXT("JOIN_SESSION"));
   JsonObj->SetNumberField(TEXT("sessionId"), SessionId);
   JsonObj->SetNumberField(TEXT("playerId"), PlayerId);
   JsonObj->SetStringField(TEXT("password"), Password);

   FString Message;
   TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Message);
   FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);

   return SendMessage(Message);
}

void NetworkClient::RequestSessionsList()
{
   if (!IsConnected())
   {
       SetLastError(ENetworkError::ConnectionFailed, TEXT("Not connected to server"));
       return;
   }

   UE_LOG(LogTemp, Warning, TEXT("Requesting sessions list as player %d"), PlayerId);

   TSharedPtr<FJsonObject> JsonObj = MakeShared<FJsonObject>();
   JsonObj->SetStringField(TEXT("type"), TEXT("GET_SESSIONS"));
   JsonObj->SetNumberField(TEXT("playerId"), PlayerId);

   FString Message;
   TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Message);
   FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);

   SendMessage(Message);
}

void NetworkClient::SendPlayerState(const FNetworkPlayerState& State)
{
   TSharedPtr<FJsonObject> JsonObj = MakeShared<FJsonObject>();
   JsonObj->SetStringField(TEXT("type"), TEXT("PLAYER_STATE"));
   JsonObj->SetNumberField(TEXT("playerId"), PlayerId);
   JsonObj->SetNumberField(TEXT("sessionId"), CurrentSessionId);

   // Position
   TSharedPtr<FJsonObject> Position = MakeShared<FJsonObject>();
   Position->SetNumberField(TEXT("x"), State.Position.X);
   Position->SetNumberField(TEXT("y"), State.Position.Y);
   Position->SetNumberField(TEXT("z"), State.Position.Z);
   JsonObj->SetObjectField(TEXT("position"), Position);

   // Rotation
   TSharedPtr<FJsonObject> Rotation = MakeShared<FJsonObject>();
   Rotation->SetNumberField(TEXT("x"), State.Rotation.Pitch);
   Rotation->SetNumberField(TEXT("y"), State.Rotation.Yaw);
   Rotation->SetNumberField(TEXT("z"), State.Rotation.Roll);
   JsonObj->SetObjectField(TEXT("rotation"), Rotation);

   JsonObj->SetBoolField(TEXT("isCrouching"), State.bIsCrouching);
   JsonObj->SetBoolField(TEXT("isWalking"), State.bIsWalking);

   // Weapon state
   TSharedPtr<FJsonObject> Weapon = MakeShared<FJsonObject>();
   Weapon->SetBoolField(TEXT("isFiring"), State.bIsFiring);
   Weapon->SetBoolField(TEXT("isReloading"), State.bIsReloading);
   Weapon->SetNumberField(TEXT("currentAmmo"), State.CurrentAmmo);
   JsonObj->SetObjectField(TEXT("weapon"), Weapon);

   FString Message;
   TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Message);
   FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);

   SendMessage(Message);
}

void NetworkClient::SendShot(const FNetworkShotInfo& ShotInfo)
{
   TSharedPtr<FJsonObject> JsonObj = MakeShared<FJsonObject>();
   JsonObj->SetStringField(TEXT("type"), TEXT("SHOT"));
   JsonObj->SetNumberField(TEXT("shooterId"), PlayerId);
   JsonObj->SetNumberField(TEXT("sessionId"), CurrentSessionId);
   JsonObj->SetNumberField(TEXT("bulletId"), ShotInfo.BulletId);

   // Start location
   TSharedPtr<FJsonObject> StartLocation = MakeShared<FJsonObject>();
   StartLocation->SetNumberField(TEXT("x"), ShotInfo.StartLocation.X);
   StartLocation->SetNumberField(TEXT("y"), ShotInfo.StartLocation.Y);
   StartLocation->SetNumberField(TEXT("z"), ShotInfo.StartLocation.Z);
   JsonObj->SetObjectField(TEXT("startLocation"), StartLocation);

   // Direction
   TSharedPtr<FJsonObject> Direction = MakeShared<FJsonObject>();
   Direction->SetNumberField(TEXT("x"), ShotInfo.Direction.X);
   Direction->SetNumberField(TEXT("y"), ShotInfo.Direction.Y);
   Direction->SetNumberField(TEXT("z"), ShotInfo.Direction.Z);
   JsonObj->SetObjectField(TEXT("direction"), Direction);

   JsonObj->SetNumberField(TEXT("speed"), ShotInfo.Speed);
   JsonObj->SetNumberField(TEXT("damage"), ShotInfo.Damage);
   JsonObj->SetNumberField(TEXT("spread"), ShotInfo.Spread);

   FString Message;
   TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Message);
   FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);

   SendMessage(Message);
}

void NetworkClient::SendHitConfirm(const FNetworkHitInfo& HitInfo)
{
   TSharedPtr<FJsonObject> JsonObj = MakeShared<FJsonObject>();
   JsonObj->SetStringField(TEXT("type"), TEXT("HIT_CONFIRM"));
   JsonObj->SetNumberField(TEXT("bulletId"), HitInfo.BulletId);
   JsonObj->SetNumberField(TEXT("sessionId"), CurrentSessionId);
   JsonObj->SetNumberField(TEXT("shooterId"), HitInfo.ShooterId);
   JsonObj->SetNumberField(TEXT("victimId"), HitInfo.VictimId);

   // Hit Location
   TSharedPtr<FJsonObject> HitLocation = MakeShared<FJsonObject>();
   HitLocation->SetNumberField(TEXT("x"), HitInfo.HitLocation.X);
   HitLocation->SetNumberField(TEXT("y"), HitInfo.HitLocation.Y);
   HitLocation->SetNumberField(TEXT("z"), HitInfo.HitLocation.Z);
   JsonObj->SetObjectField(TEXT("hitLocation"), HitLocation);

   // Hit Normal
   TSharedPtr<FJsonObject> HitNormal = MakeShared<FJsonObject>();
   HitNormal->SetNumberField(TEXT("x"), HitInfo.HitNormal.X);
   HitNormal->SetNumberField(TEXT("y"), HitInfo.HitNormal.Y);
   HitNormal->SetNumberField(TEXT("z"), HitInfo.HitNormal.Z);
   JsonObj->SetObjectField(TEXT("hitNormal"), HitNormal);

   JsonObj->SetNumberField(TEXT("damageTaken"), HitInfo.DamageTaken);

   FString Message;
   TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Message);
   FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);

   SendMessage(Message);
}



void NetworkClient::SendGrenadeThrow(const FNetworkGrenadeInfo& GrenadeInfo)
{
    TSharedPtr<FJsonObject> JsonObj = MakeShared<FJsonObject>();
    JsonObj->SetStringField(TEXT("type"), TEXT("GRENADE_THROW"));
    JsonObj->SetNumberField(TEXT("throwerId"), PlayerId);
    JsonObj->SetNumberField(TEXT("sessionId"), CurrentSessionId);

    // Location
    TSharedPtr<FJsonObject> Location = MakeShared<FJsonObject>();
    Location->SetNumberField(TEXT("x"), GrenadeInfo.Location.X);
    Location->SetNumberField(TEXT("y"), GrenadeInfo.Location.Y);
    Location->SetNumberField(TEXT("z"), GrenadeInfo.Location.Z);
    JsonObj->SetObjectField(TEXT("location"), Location);

    // Rotation
    TSharedPtr<FJsonObject> Rotation = MakeShared<FJsonObject>();
    Rotation->SetNumberField(TEXT("pitch"), GrenadeInfo.Rotation.Pitch);
    Rotation->SetNumberField(TEXT("yaw"), GrenadeInfo.Rotation.Yaw);
    Rotation->SetNumberField(TEXT("roll"), GrenadeInfo.Rotation.Roll);
    JsonObj->SetObjectField(TEXT("rotation"), Rotation);

    // Velocity
    TSharedPtr<FJsonObject> Velocity = MakeShared<FJsonObject>();
    Velocity->SetNumberField(TEXT("x"), GrenadeInfo.Velocity.X);
    Velocity->SetNumberField(TEXT("y"), GrenadeInfo.Velocity.Y);
    Velocity->SetNumberField(TEXT("z"), GrenadeInfo.Velocity.Z);
    JsonObj->SetObjectField(TEXT("velocity"), Velocity);

    FString Message;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Message);
    FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);

    SendMessage(Message);
}


bool NetworkClient::SendMessage(const FString& Message)
{
   if (!bConnected || !Socket)
   {
       return false;
   }

   TArray<uint8> Data;
   Data.Append((uint8*)TCHAR_TO_UTF8(*Message), Message.Len());
   Data.Add(0); // Null terminator

   int32 BytesSent = 0;
   return Socket->Send(Data.GetData(), Data.Num(), BytesSent);
}

void NetworkClient::StartReceiveThread()
{
   AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]()
   {
       ReceiveLoop();
   });
}

void NetworkClient::ReceiveLoop()
{
   TArray<uint8> ReceivedData;
   ReceivedData.SetNumUninitialized(1024);

   while (bConnected && Socket)
   {
       int32 BytesRead = 0;
       if (Socket->Recv(ReceivedData.GetData(), ReceivedData.Num(), BytesRead))
       {
           if (BytesRead > 0)
           {
               FString ReceivedMessage = UTF8_TO_TCHAR(ReceivedData.GetData());
               AsyncTask(ENamedThreads::GameThread, [this, ReceivedMessage]()
               {
                   HandleMessage(ReceivedMessage);
               });
           }
       }
       else
       {
           break;
       }
   }
}

void NetworkClient::HandleMessage(const FString& Message)
{
   TSharedPtr<FJsonObject> JsonObj;
   TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);
   if (!FJsonSerializer::Deserialize(Reader, JsonObj))
   {
       UE_LOG(LogTemp, Error, TEXT("Failed to parse message: %s"), *Message);
       return;
   }

   FString Type = JsonObj->GetStringField(TEXT("type"));
   UE_LOG(LogTemp, Warning, TEXT("Received message type: %s"), *Type);

   if (Type == TEXT("SESSION_CREATED"))
   {
       int32 SessionId = JsonObj->GetNumberField(TEXT("sessionId"));
       bool Success = JsonObj->GetBoolField(TEXT("success"));
       
       UE_LOG(LogTemp, Warning, TEXT("Session created response - Success: %d, SessionID: %d"), 
           Success ? 1 : 0, SessionId);
           
       if (Success)
       {
           CurrentSessionId = SessionId;
           OnSessionCreated.Broadcast(SessionId);
       }
       else
       {
           SetLastError(ENetworkError::SessionCreationFailed, TEXT("Failed to create session"));
       }
   }
   else if (Type == TEXT("SESSION_JOINED"))
   {
       bool Success = JsonObj->GetBoolField(TEXT("success"));
       int32 SessionId = JsonObj->GetNumberField(TEXT("sessionId"));
       
       UE_LOG(LogTemp, Warning, TEXT("Session joined response - Success: %d, SessionID: %d"), 
           Success ? 1 : 0, SessionId);

       if (Success)
       {
           CurrentSessionId = SessionId;
       }
       
       OnSessionJoined.Broadcast(Success, SessionId);
   }
   else if (Type == TEXT("SESSIONS_LIST"))
   {
       TArray<FSessionInfo> Sessions;
       const TArray<TSharedPtr<FJsonValue>>* SessionsArray;
       
       UE_LOG(LogTemp, Warning, TEXT("Received SESSIONS_LIST response from server"));
       
       if (JsonObj->TryGetArrayField(TEXT("sessions"), SessionsArray))
       {
           UE_LOG(LogTemp, Warning, TEXT("Found %d sessions in response"), SessionsArray->Num());
           
           for (const auto& SessionValue : *SessionsArray)
           {
               const TSharedPtr<FJsonObject>& SessionObj = SessionValue->AsObject();
               FSessionInfo SessionInfo;
               
               SessionInfo.SessionId = SessionObj->GetNumberField(TEXT("sessionId"));
               SessionInfo.MapType = static_cast<EGameMapType>(SessionObj->GetNumberField(TEXT("mapType")));
               SessionInfo.CurrentPlayers = SessionObj->GetNumberField(TEXT("currentPlayers"));
               SessionInfo.HasPassword = SessionObj->GetBoolField(TEXT("hasPassword"));
               SessionInfo.SessionName = SessionObj->GetStringField(TEXT("sessionName"));

               UE_LOG(LogTemp, Warning, TEXT("Found session: ID=%d, MapType=%d, Players=%d"),
                   SessionInfo.SessionId,
                   static_cast<int32>(SessionInfo.MapType),
                   SessionInfo.CurrentPlayers);
               
               Sessions.Add(SessionInfo);
           }
           
           OnSessionsListReceived.Broadcast(Sessions);
       }
       else
       {
           UE_LOG(LogTemp, Error, TEXT("Failed to parse sessions array from response"));
           FString JsonString;
           TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
           FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);
           UE_LOG(LogTemp, Warning, TEXT("Raw response: %s"), *JsonString);
       }
   }
   else if (Type == TEXT("PLAYER_STATE"))
   {
       FNetworkPlayerState State;
       State.PlayerId = JsonObj->GetNumberField(TEXT("playerId"));
       
       const TSharedPtr<FJsonObject>* PositionObj;
       if (JsonObj->TryGetObjectField(TEXT("position"), PositionObj))
       {
           State.Position = FVector(
               (*PositionObj)->GetNumberField(TEXT("x")),
               (*PositionObj)->GetNumberField(TEXT("y")),
               (*PositionObj)->GetNumberField(TEXT("z"))
           );
       }

       const TSharedPtr<FJsonObject>* RotationObj;
       if (JsonObj->TryGetObjectField(TEXT("rotation"), RotationObj))
       {
           State.Rotation = FRotator(
               (*RotationObj)->GetNumberField(TEXT("x")),
               (*RotationObj)->GetNumberField(TEXT("y")),
               (*RotationObj)->GetNumberField(TEXT("z"))
           );
       }

       State.bIsCrouching = JsonObj->GetBoolField(TEXT("isCrouching"));
       State.bIsWalking = JsonObj->GetBoolField(TEXT("isWalking"));

       const TSharedPtr<FJsonObject>* WeaponObj;
       if (JsonObj->TryGetObjectField(TEXT("weapon"), WeaponObj))
       {
           State.bIsFiring = (*WeaponObj)->GetBoolField(TEXT("isFiring"));
           State.bIsReloading = (*WeaponObj)->GetBoolField(TEXT("isReloading"));
           State.CurrentAmmo = (*WeaponObj)->GetNumberField(TEXT("currentAmmo"));
       }

       OnPlayerStateReceived.Broadcast(State);
   }
   else if (Type == TEXT("SHOT"))
   {
       FNetworkShotInfo ShotInfo;
       ShotInfo.ShooterId = JsonObj->GetNumberField(TEXT("shooterId"));
       ShotInfo.BulletId = JsonObj->GetNumberField(TEXT("bulletId"));
       
       const TSharedPtr<FJsonObject>* StartLocationObj;
       if (JsonObj->TryGetObjectField(TEXT("startLocation"), StartLocationObj))
       {
           ShotInfo.StartLocation = FVector(
               (*StartLocationObj)->GetNumberField(TEXT("x")),
               (*StartLocationObj)->GetNumberField(TEXT("y")),
               (*StartLocationObj)->GetNumberField(TEXT("z"))
           );
       }

       const TSharedPtr<FJsonObject>* DirectionObj;
       if (JsonObj->TryGetObjectField(TEXT("direction"), DirectionObj))
       {
           ShotInfo.Direction = FVector(
               (*DirectionObj)->GetNumberField(TEXT("x")),
               (*DirectionObj)->GetNumberField(TEXT("y")),
               (*DirectionObj)->GetNumberField(TEXT("z"))
           );
       }

       ShotInfo.Speed = JsonObj->GetNumberField(TEXT("speed"));
       ShotInfo.Damage = JsonObj->GetNumberField(TEXT("damage"));
       ShotInfo.Spread = JsonObj->GetNumberField(TEXT("spread"));

       OnShotReceived.Broadcast(ShotInfo);
   }
   else
   {
       UE_LOG(LogTemp, Warning, TEXT("Unhandled message type: %s"), *Type);
   }
}

void NetworkClient::ProcessError(int32 ErrorCode, const FString& ErrorMessage)
{
   AsyncTask(ENamedThreads::GameThread, [this, ErrorCode, ErrorMessage]()
   {
       OnError.Broadcast(ErrorCode, ErrorMessage);
   });
}

void NetworkClient::SetLastError(ENetworkError ErrorCode, const FString& Message)
{
   LastErrorCode = ErrorCode;
   LastErrorMessage = Message;
   
   UE_LOG(LogTemp, Error, TEXT("Network Error: %s"), *Message);
   OnError.Broadcast(static_cast<int32>(ErrorCode), Message);
}

bool NetworkClient::ValidateSessionParameters(EGameMapType MapType, const FString& Password)
{
   if (static_cast<int32>(MapType) < 0 || static_cast<int32>(MapType) > 2)
   {
       return false;
   }
   
   if (!Password.IsEmpty() && Password.Len() > 20)
   {
       return false;
   }
   
   return true;
}