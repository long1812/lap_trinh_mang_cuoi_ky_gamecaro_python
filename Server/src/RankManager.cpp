#include "Game/RankManager.h"

using namespace Server;
using namespace System::Collections::Generic;

RankManager::RankManager()
{
    ranks = gcnew Dictionary<String^, RankInfo^>();
}

void RankManager::UpdateRank(String^ playerName, bool win)
{
    if(!ranks->ContainsKey(playerName))
        ranks[playerName] = gcnew RankInfo(playerName);

    if(win) ranks[playerName]->AddWin();
    else ranks[playerName]->AddLoss();
}

RankInfo^ RankManager::GetRank(String^ playerName)
{
    if(ranks->ContainsKey(playerName))
        return ranks[playerName];
    return nullptr;
}
