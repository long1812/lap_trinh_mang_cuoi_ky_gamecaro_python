#pragma once
#include "Board.h"
using namespace System;
using namespace System::Windows::Forms;
using namespace System::Drawing;
using namespace System::Collections::Generic;

namespace Client {
namespace Game {

    public ref class GameRenderer
    {
    public:
        GameRenderer(Panel^ canvas, Board^ board)
        {
            _canvas = canvas;
            _board = board;
            WinningCells = nullptr;

            _canvas->Paint += gcnew PaintEventHandler(this, &GameRenderer::OnPaint);
        }

        List<Point>^ WinningCells;

        // chuyển tọa độ pixel -> ô
        Nullable<Point> PointToCell(Point pt)
        {
            int size = _canvas->Width / _board->Cols;
            int col = pt.X / size;
            int row = pt.Y / size;
            if (row < 0 || row >= _board->Rows || col < 0 || col >= _board->Cols)
                return Nullable<Point>();
            return Point(row, col);
        }

        void Refresh() {
            _canvas->Invalidate();
        }

    private:
        Panel^ _canvas;
        Board^ _board;

        void OnPaint(Object^ sender, PaintEventArgs^ e)
        {
            Graphics^ g = e->Graphics;
            int w = _canvas->Width;
            int h = _canvas->Height;
            int rows = _board->Rows;
            int cols = _board->Cols;
            int cellW = w / cols;
            int cellH = h / rows;

            Pen^ pen = gcnew Pen(Color::Black);

            // vẽ lưới
            for (int r = 0; r <= rows; r++)
                g->DrawLine(pen, 0, r * cellH, w, r * cellH);
            for (int c = 0; c <= cols; c++)
                g->DrawLine(pen, c * cellW, 0, c * cellW, h);

            // vẽ quân cờ
            for (int r = 0; r < rows; r++)
            {
                for (int c = 0; c < cols; c++)
                {
                    String^ mark = _board->Cells[r, c];
                    if (mark == nullptr) continue;

                    Rectangle rect(c * cellW, r * cellH, cellW, cellH);
                    if (mark == "X")
                        g->DrawLine(gcnew Pen(Color::Red, 2), rect.Location, Point(rect.Right, rect.Bottom));
                    else
                        g->DrawEllipse(gcnew Pen(Color::Blue, 2), rect);
                }
            }

            // vẽ đường thắng nếu có
            if (WinningCells != nullptr && WinningCells->Count >= 2)
            {
                Pen^ winPen = gcnew Pen(Color::Red, 3);
                Point start = WinningCells[0];
                Point end = WinningCells[WinningCells->Count - 1];
                g->DrawLine(winPen, start.X * cellW + cellW/2, start.Y * cellH + cellH/2,
                            end.X * cellW + cellW/2, end.Y * cellH + cellH/2);
            }
        }
    };
}}
