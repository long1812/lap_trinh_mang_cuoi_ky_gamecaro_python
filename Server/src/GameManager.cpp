#include "Game/GameManager.h"

using namespace Server;
using namespace System::Collections::Generic;

GameManager::GameManager()
{
    rooms = gcnew List<GameRoom^>();
}

GameRoom^ GameManager::CreateRoom(String^ roomName)
{
    GameRoom^ room = gcnew GameRoom(roomName);
    rooms->Add(room);
    return room;
}

GameRoom^ GameManager::GetRoom(String^ roomName)
{
    for each(GameRoom^ r in rooms)
    {
        if(r->Name == roomName) return r;
    }
    return nullptr;
}
