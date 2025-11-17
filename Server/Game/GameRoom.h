#pragma once
#ifndef GAMEROOM_H
#define GAMEROOM_H

#include "CaroLogic.h"
using namespace System;
using namespace System::Collections::Generic;

namespace Server {

    public ref class GameRoom
    {
    public:
        String^ Name;
        List<Tuple<int,int>^>^ MovesX;
        List<Tuple<int,int>^>^ MovesO;
        String^ Winner;

        GameRoom(String^ name);
        bool PlaceMove(int row, int col, String^ mark);
    };

}

#endif
