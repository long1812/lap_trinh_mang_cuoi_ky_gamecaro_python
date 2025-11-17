#pragma once
#ifndef RANKINFO_H
#define RANKINFO_H

using namespace System;

namespace Server {

    public ref class RankInfo
    {
    public:
        String^ PlayerName;
        int Wins;
        int Losses;
        int Score;

        RankInfo(String^ name);
        void AddWin();
        void AddLoss();
    };

}

#endif
