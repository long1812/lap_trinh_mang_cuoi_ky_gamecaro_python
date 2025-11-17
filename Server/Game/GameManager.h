#pragma once
#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include "GameRoom.h"
using namespace System;
using namespace System::Collections::Generic;

namespace Server {

    public ref class GameManager
    {
    private:
        List<GameRoom^>^ rooms;

    public:
        GameManager();
        GameRoom^ CreateRoom(String^ roomName);
        GameRoom^ GetRoom(String^ roomName);
    };

}

#endif
