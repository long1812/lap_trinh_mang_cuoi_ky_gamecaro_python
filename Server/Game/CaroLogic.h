#pragma once
#ifndef CAROLOGIC_H
#define CAROLOGIC_H

using namespace System;
using namespace System::Collections::Generic;

namespace Server {

    public ref class CaroLogic
    {
    public:
        // Kiểm tra thắng cờ
        static bool CheckWinner(List<Tuple<int,int>^>^ moves, String^ mark);
    };

}

#endif
