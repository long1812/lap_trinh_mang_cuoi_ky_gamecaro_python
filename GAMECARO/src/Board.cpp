#include "Game/Board.h"

using namespace Client::Game;

Board::Board()
{
    Rows = 15;
    Cols = 15;
    Cells = gcnew array<String^, 2>(Rows, Cols);
    NextTurn = "X";
}

void Board::Reset()
{
    for (int r = 0; r < Rows; r++)
        for (int c = 0; c < Cols; c++)
            Cells[r,c] = nullptr;
    NextTurn = "X";
    Winner = nullptr;
}

bool Board::PlaceMark(int row, int col, String^ mark)
{
    if (Cells[row, col] != nullptr) return false;
    Cells[row, col] = mark;
    NextTurn = (mark == "X") ? "O" : "X";

    if (CheckWinner(row, col, mark))
        Winner = mark;
    return true;
}

bool Board::CheckWinner(int row, int col, String^ mark)
{
    int directions[4][2] = { {0,1},{1,0},{1,1},{1,-1} };
    for (int d=0; d<4; d++)
    {
        int count = 1;
        int dr = directions[d][0];
        int dc = directions[d][1];

        int r = row + dr;
        int c = col + dc;
        while(r>=0 && r<Rows && c>=0 && c<Cols && Cells[r,c]==mark)
        { count++; r+=dr; c+=dc; }

        r = row - dr;
        c = col - dc;
        while(r>=0 && r<Rows && c>=0 && c<Cols && Cells[r,c]==mark)
        { count++; r-=dr; c-=dc; }

        if(count>=5) return true;
    }
    return false;
}
