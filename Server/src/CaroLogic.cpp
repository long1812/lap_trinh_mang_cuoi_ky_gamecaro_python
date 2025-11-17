#include "Game/CaroLogic.h"

using namespace Server;
using namespace System::Collections::Generic;

bool CaroLogic::CheckWinner(List<Tuple<int,int>^>^ moves, String^ mark)
{
    if(moves->Count < 5) return false;

    array<int,2>^ directions = gcnew array<int,2> {0,1, 1,0, 1,1, 1,-1};

    for each(auto move in moves)
    {
        int row = move->Item1;
        int col = move->Item2;

        for(int d=0; d<4; d++)
        {
            int dr = directions[d,0];
            int dc = directions[d,1];
            int count = 1;

            int r = row+dr, c = col+dc;
            while(moves->Exists(gcnew Predicate<Tuple<int,int>^>([r,c](Tuple<int,int>^ t){ return t->Item1==r && t->Item2==c; })))
            {
                count++; r+=dr; c+=dc;
            }

            r = row-dr; c = col-dc;
            while(moves->Exists(gcnew Predicate<Tuple<int,int>^>([r,c](Tuple<int,int>^ t){ return t->Item1==r && t->Item2==c; })))
            {
                count++; r-=dr; c-=dc;
            }

            if(count>=5) return true;
        }
    }

    return false;
}
