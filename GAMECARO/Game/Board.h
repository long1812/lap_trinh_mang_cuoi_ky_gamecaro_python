#pragma once
#include <cliext/list>

using namespace System;
using namespace System::Drawing;
using namespace cliext;

namespace Client {
namespace Game {

    public ref class Board
    {
    public:
        Board()
        {
            Rows = 15;
            Cols = 15;
            Cells = gcnew array<String^, 2>(Rows, Cols);
            NextTurn = "X";
        }

        property int Rows;
        property int Cols;
        property String^ NextTurn;
        property String^ Winner;
        property array<String^, 2>^ Cells;

        void Reset()
        {
            for (int r = 0; r < Rows; r++)
                for (int c = 0; c < Cols; c++)
                    Cells[r, c] = nullptr;
            NextTurn = "X";
            Winner = nullptr;
        }

        bool PlaceMark(int row, int col, String^ mark)
        {
            if (Cells[row, col] != nullptr) return false;
            Cells[row, col] = mark;

            NextTurn = (mark == "X") ? "O" : "X";

            // kiểm tra thắng
            if (CheckWinner(row, col, mark))
            {
                Winner = mark;
            }

            return true;
        }

    private:
        bool CheckWinner(int row, int col, String^ mark)
        {
            // kiểm tra 5 ô theo hàng, cột, chéo
            int directions[4][2] = { {0,1},{1,0},{1,1},{1,-1} };
            for (int d = 0; d < 4; d++)
            {
                int count = 1;
                int dr = directions[d][0];
                int dc = directions[d][1];

                // đi theo hướng dương
                int r = row + dr;
                int c = col + dc;
                while (r >= 0 && r < Rows && c >= 0 && c < Cols && Cells[r, c] == mark)
                {
                    count++;
                    r += dr;
                    c += dc;
                }

                // đi theo hướng âm
                r = row - dr;
                c = col - dc;
                while (r >= 0 && r < Rows && c >= 0 && c < Cols && Cells[r, c] == mark)
                {
                    count++;
                    r -= dr;
                    c -= dc;
                }

                if (count >= 5)
                    return true;
            }
            return false;
        }
    };
}}
