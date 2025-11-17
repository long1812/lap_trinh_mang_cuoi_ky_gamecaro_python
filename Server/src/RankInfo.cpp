#include "Model/RankInfo.h"

using namespace Server;

RankInfo::RankInfo(String^ name)
{
    PlayerName = name;
    Wins = 0;
    Losses = 0;
    Score = 0;
}

void RankInfo::AddWin()
{
    Wins++;
    Score += 10;
}

void RankInfo::AddLoss()
{
    Losses++;
    Score -= 5;
}
