#include "Game/GameRoom.h"

using namespace Server;

GameRoom::GameRoom(String^ name)
{
    Name = name;
    MovesX = gcnew List<Tuple<int,int>^>();
    MovesO = gcnew List<Tuple<int,int>^>();
    Winner = nullptr;
}

bool GameRoom::PlaceMove(int row, int col, String^ mark)
{
    if(Winner != nullptr) return false;

    auto move = gcnew Tuple<int,int>(row,col);
    if(mark=="X") MovesX->Add(move);
    else MovesO->Add(move);

    if(CaroLogic::CheckWinner((mark=="X"?MovesX:MovesO), mark))
        Winner = mark;

    return true;
}
