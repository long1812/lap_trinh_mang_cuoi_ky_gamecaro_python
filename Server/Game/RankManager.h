#pragma once
#ifndef RANKMANAGER_H
#define RANKMANAGER_H

#include "RankInfo.h"
using namespace System;
using namespace System::Collections::Generic;

namespace Server {

    public ref class RankManager
    {
    private:
        Dictionary<String^, RankInfo^>^ ranks;

    public:
        RankManager();
        void UpdateRank(String^ playerName, bool win);
        RankInfo^ GetRank(String^ playerName);
    };

}

#endif
