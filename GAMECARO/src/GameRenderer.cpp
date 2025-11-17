// ...existing code...
#include "Game/GameRenderer.h"

using namespace Client::Game;

GameRenderer::GameRenderer(Panel^ canvas, Board^ board)
{
    _canvas = canvas;
    _board = board;
    WinningCells = nullptr;
    _canvas->Paint += gcnew PaintEventHandler(this, &GameRenderer::OnPaint);
}

Nullable<Point> GameRenderer::PointToCell(Point pt)
{
    // bảo vệ chia cho 0 và trả Point với X=col, Y=row (consistent)
    if (_board == nullptr || _board->Cols <= 0 || _board->Rows <= 0 || _canvas == nullptr) 
        return Nullable<Point>();

    int cols = _board->Cols;
    int rows = _board->Rows;
    int cellW = _canvas->Width / cols;
    int cellH = _canvas->Height / rows;
    if (cellW <= 0 || cellH <= 0) return Nullable<Point>();

    int col = pt.X / cellW;
    int row = pt.Y / cellH;
    if (row < 0 || row >= rows || col < 0 || col >= cols) return Nullable<Point>();

    // X = column, Y = row
    return Point(col, row);
}

void GameRenderer::Refresh()
{
    if (_canvas != nullptr) _canvas->Invalidate();
}

void GameRenderer::OnPaint(Object^ sender, PaintEventArgs^ e)
{
    if (_canvas == nullptr || _board == nullptr) return;

    Graphics^ g = e->Graphics;
    int w = _canvas->Width;
    int h = _canvas->Height;
    int rows = _board->Rows;
    int cols = _board->Cols;
    if (rows <= 0 || cols <= 0) return;

    int cellW = w / cols;
    int cellH = h / rows;
    if (cellW <= 0 || cellH <= 0) return;

    Pen^ gridPen = gcnew Pen(Color::Black);
    // draw horizontal and vertical lines
    for (int r = 0; r <= rows; r++)
        g->DrawLine(gridPen, 0, r * cellH, w, r * cellH);
    for (int c = 0; c <= cols; c++)
        g->DrawLine(gridPen, c * cellW, 0, c * cellW, h);

    // draw marks
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            String^ mark = _board->Cells[r, c];
            if (mark == nullptr) continue;

            Rectangle rect(c * cellW, r * cellH, cellW, cellH);

            if (mark->Equals("X"))
            {
                Pen^ xPen = gcnew Pen(Color::Red, 2);
                // both diagonals
                g->DrawLine(xPen, rect.Left, rect.Top, rect.Right, rect.Bottom);
                g->DrawLine(xPen, rect.Left, rect.Bottom, rect.Right, rect.Top);
            }
            else // assume "O" or other -> draw circle/ellipse with padding
            {
                Pen^ oPen = gcnew Pen(Color::Blue, 2);
                int pad = Math::Max(2, Math::Min(cellW, cellH) / 8);
                Rectangle inset(rect.X + pad, rect.Y + pad, Math::Max(0, rect.Width - 2 * pad), Math::Max(0, rect.Height - 2 * pad));
                if (inset.Width > 0 && inset.Height > 0)
                    g->DrawEllipse(oPen, inset);
            }
        }
    }

    // draw winning line if any (WinningCells: X=col, Y=row)
    if (WinningCells != nullptr && WinningCells->Count >= 2)
    {
        Pen^ winPen = gcnew Pen(Color::Red, 3);
        Point start = WinningCells[0];
        Point end = WinningCells[WinningCells->Count - 1];
        int sx = start.X * cellW + cellW / 2;
        int sy = start.Y * cellH + cellH / 2;
        int ex = end.X * cellW + cellW / 2;
        int ey = end.Y * cellH + cellH / 2;
        g->DrawLine(winPen, sx, sy, ex, ey);
    }
}
// ...existing code...