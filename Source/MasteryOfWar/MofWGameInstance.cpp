#include "MofWGameInstance.h"
#include "MasteryOfWarCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "WeaponSystem.h"
#include "Grenade.h"


UMasteryOfWarGameInstance::UMasteryOfWarGameInstance(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , CurrentUserId(-1)
    , CurrentGameMode(EGameMapType::None)
{
}


bool UMasteryOfWarGameInstance::InitializeNetworking(const FString& IPAddress, int32 Port)
{
    if (!NetworkConnection)
    {
        NetworkConnection = MakeUnique<NetworkClient>();
        NetworkConnection->SetUserId(CurrentUserId); // Установить User ID
        
        // Subscribe to network events
        NetworkConnection->OnError.AddUObject(this, &UMasteryOfWarGameInstance::HandleNetworkError);
        NetworkConnection->OnSessionCreated.AddUObject(this, &UMasteryOfWarGameInstance::HandleSessionCreated);
        NetworkConnection->OnPlayerStateReceived.AddUObject(this, &UMasteryOfWarGameInstance::HandlePlayerState);
        NetworkConnection->OnShotReceived.AddUObject(this, &UMasteryOfWarGameInstance::HandleShot);
        NetworkConnection->OnGrenadeThrowReceived.AddUObject(this, &UMasteryOfWarGameInstance::HandleGrenadeThrow);
    }
    
    return NetworkConnection->Connect(IPAddress, Port);
}



bool UMasteryOfWarGameInstance::CreateGameSession(EGameMapType MapType, const FString& Password)
{
    if (!NetworkConnection || !NetworkConnection->IsConnected())
    {
        return false;
    }
    
    return NetworkConnection->CreateSession(MapType, Password);
}

bool UMasteryOfWarGameInstance::JoinGameSession(int32 SessionId, const FString& Password)
{
    if (!NetworkConnection || !NetworkConnection->IsConnected())
    {
        return false;
    }
    
    return NetworkConnection->JoinSession(SessionId, Password);
}

void UMasteryOfWarGameInstance::DisconnectFromServer()
{
    if (NetworkConnection)
    {
        NetworkConnection->Disconnect();
    }
}

bool UMasteryOfWarGameInstance::IsConnectedToServer() const
{
    return NetworkConnection && NetworkConnection->IsConnected();
}

void UMasteryOfWarGameInstance::HandleNetworkError(int32 ErrorCode, const FString& ErrorMessage)
{
    OnNetworkError.Broadcast(ErrorCode, ErrorMessage);
}

void UMasteryOfWarGameInstance::HandleSessionCreated(int32 SessionId)
{
    OnNetworkCreatedSession.Broadcast(SessionId);
}

void UMasteryOfWarGameInstance::HandlePlayerState(const FNetworkPlayerState& State)
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> Characters;
        UGameplayStatics::GetAllActorsOfClass(World, AMasteryOfWarCharacter::StaticClass(), Characters);
        
        for (AActor* Actor : Characters)
        {
            if (AMasteryOfWarCharacter* Character = Cast<AMasteryOfWarCharacter>(Actor))
            {
                if (Character->GetPlayerId() == State.PlayerId && 
                    Character->GetPlayerId() != NetworkConnection->GetPlayerId())
                {
                    Character->UpdateFromNetworkState(State);
                    break;
                }
            }
        }
    }
}


void UMasteryOfWarGameInstance::SetCurrentUserId(int32 UserId)
{
    CurrentUserId = UserId;
    
    UE_LOG(LogTemp, Warning, TEXT("Current user ID set to: %d"), CurrentUserId);
}

void UMasteryOfWarGameInstance::HandleShot(const FNetworkShotInfo& ShotInfo)
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> Characters;
        UGameplayStatics::GetAllActorsOfClass(World, AMasteryOfWarCharacter::StaticClass(), Characters);
        
        for (AActor* Actor : Characters)
        {
            if (AMasteryOfWarCharacter* Character = Cast<AMasteryOfWarCharacter>(Actor))
            {
                if (Character->GetPlayerId() == ShotInfo.ShooterId)
                {
                    if (AWeapon* Weapon = Character->GetCurrentWeapon())
                    {
                        Weapon->SimulateShot(ShotInfo);
                    }
                    break;
                }
            }
        }
    }
}

void UMasteryOfWarGameInstance::HandleGrenadeThrow(const FNetworkGrenadeInfo& GrenadeInfo)
{
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> Characters;
        UGameplayStatics::GetAllActorsOfClass(World, AMasteryOfWarCharacter::StaticClass(), Characters);
        
        for (AActor* Actor : Characters)
        {
            if (AMasteryOfWarCharacter* Character = Cast<AMasteryOfWarCharacter>(Actor))
            {
                if (Character->GetPlayerId() == GrenadeInfo.ThrowerId)
                {
                    if (AGrenade* Grenade = Cast<AGrenade>(Character->GetCurrentWeapon()))
                    {
                        Grenade->SimulateThrow(GrenadeInfo);
                    }
                    break;
                }
            }
        }
    }
}