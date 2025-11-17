#pragma once
#include "FormLogin.h"
#include "GameRenderer.h"
#include "GameLoop.h"
#include "ConnectToServer.h"
#include "Board.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;
using namespace System::Collections::Generic;

namespace Client {
namespace Forms {

    public ref class FormMain : public Form
    {
    public:
        FormMain(String^ playerName)
        {
            _playerName = playerName;
            InitializeComponent();

            this->Text = "Caro Online 15x15";
            this->ClientSize = System::Drawing::Size(800, 620);
            this->DoubleBuffered = true;

            // ====== PANEL TOP ======
            Panel^ top = gcnew Panel();
            top->Dock = DockStyle::Top;
            top->Height = 40;
            top->Padding = Padding(8);
            top->BackColor = Color::LightGray;

            _lblTurn = gcnew Label();
            _lblTurn->AutoSize = true;
            _lblTurn->Text = "NextTurn: ...";
            _lblTurn->Font = gcnew Font("Arial", 12, FontStyle::Bold);

            _lblMark = gcnew Label();
            _lblMark->AutoSize = true;
            _lblMark->Left = 180;
            _lblMark->Text = String::Format("Player: {0}", _playerName);
            _lblMark->Font = gcnew Font("Arial", 12, FontStyle::Bold);

            _lblTimer = gcnew Label();
            _lblTimer->AutoSize = true;
            _lblTimer->Left = 300;
            _lblTimer->Text = "⏱️ 30s";
            _lblTimer->Font = gcnew Font("Arial", 12, FontStyle::Bold);

            _lblScore = gcnew Label();
            _lblScore->AutoSize = true;
            _lblScore->Left = 420;
            _lblScore->Text = "Rank: ...";
            _lblScore->Font = gcnew Font("Arial", 12, FontStyle::Bold);

            _lblWins = gcnew Label();
            _lblWins->AutoSize = true;
            _lblWins->Left = 520;
            _lblWins->Text = "W: 0";
            _lblWins->Font = gcnew Font("Arial", 12, FontStyle::Bold);

            _lblLosses = gcnew Label();
            _lblLosses->AutoSize = true;
            _lblLosses->Left = 600;
            _lblLosses->Text = "L: 0";
            _lblLosses->Font = gcnew Font("Arial", 12, FontStyle::Bold);

            top->Controls->Add(_lblTurn);
            top->Controls->Add(_lblMark);
            top->Controls->Add(_lblTimer);
            top->Controls->Add(_lblScore);
            top->Controls->Add(_lblWins);
            top->Controls->Add(_lblLosses);
            this->Controls->Add(top);

            // ====== PANEL CANVAS ======
            _canvas = gcnew Panel();
            _canvas->Dock = DockStyle::Left;
            _canvas->Width = 380;
            _canvas->BackColor = Color::White;
            this->Controls->Add(_canvas);
            _canvas->GetType()->GetProperty("DoubleBuffered",
                System::Reflection::BindingFlags::NonPublic | System::Reflection::BindingFlags::Instance
            )->SetValue(_canvas, true, nullptr);

            // ====== GAME RENDER + LOOP ======
            _board = gcnew Board();
            _renderer = gcnew GameRenderer(_canvas, _board);
            _loop = gcnew GameLoop(this);
            _loop->Start();

            // ====== NETWORK ======
            _net = gcnew ConnectToServer("127.0.0.1", 9000, _board);

            _net->OnBoardChanged += gcnew Action(this, &FormMain::OnBoardChanged);
            _net->OnWinner += gcnew Action<String^>(this, &FormMain::OnWinner);
            _net->OnInitReceived += gcnew Action<String^>(this, &FormMain::OnInitReceived);
            _net->OnTimerUpdate += gcnew Action<int, String^>(this, &FormMain::OnTimerUpdate);
            _net->OnReset += gcnew Action(this, &FormMain::OnReset);
            _net->OnRankUpdate += gcnew Action<int, int, int>(this, &FormMain::OnRankUpdate);
            _net->OnChatReceived += gcnew Action<String^, String^>(this, &FormMain::OnChatReceived);

            _net->ConnectAndJoin(_playerName);

            // ====== CLICK CANVAS ======
            _canvas->MouseClick += gcnew MouseEventHandler(this, &FormMain::Canvas_MouseClick);
        }

    private:
        // ===== FIELDS =====
        String^ _playerName;
        Board^ _board;
        GameRenderer^ _renderer;
        GameLoop^ _loop;
        ConnectToServer^ _net;

        Panel^ _canvas;
        Label^ _lblTurn;
        Label^ _lblMark;
        Label^ _lblTimer;
        Label^ _lblScore;
        Label^ _lblWins;
        Label^ _lblLosses;

        // ===== METHODS =====
        void OnBoardChanged() {
            this->BeginInvoke(gcnew Action(this, &FormMain::UpdateBoard));
        }

        void OnWinner(String^ w) {
            this->BeginInvoke(gcnew Action(gcnew Action([w, this]() {
                MessageBox::Show(String::Format("{0} thắng!", w), "🎉 Kết quả", MessageBoxButtons::OK, MessageBoxIcon::Information);
            })));
        }

        void OnInitReceived(String^ mark) {
            this->BeginInvoke(gcnew Action(gcnew Action([this, mark]() {
                _lblMark->Text = String::Format("Player: {0} ({1})", _playerName, mark);
            })));
        }

        void OnTimerUpdate(int sec, String^ turn) {
            this->BeginInvoke(gcnew Action(gcnew Action([this, sec, turn]() {
                _lblTimer->Text = String::Format("⏱️ {0}: {1}s", turn, sec);
                _lblTimer->ForeColor = sec <= 5 ? Color::Red : Color::Black;
            })));
        }

        void OnReset() {
            this->BeginInvoke(gcnew Action(gcnew Action([this]() {
                _lblTurn->Text = "NextTurn: X";
                _lblTimer->Text = "⏱️ 30s";
                ShowWinningLine(nullptr);
                _renderer->Refresh();
            })));
        }

        void OnRankUpdate(int score, int wins, int losses) {
            this->BeginInvoke(gcnew Action(gcnew Action([this, score, wins, losses]() {
                _lblScore->Text = String::Format("Rank: {0}", score);
                _lblWins->Text = String::Format("W: {0}", wins);
                _lblLosses->Text = String::Format("L: {0}", losses);
            })));
        }

        void OnChatReceived(String^ player, String^ message) {
            this->BeginInvoke(gcnew Action(gcnew Action([this, player, message]() {
                if (player == _playerName) return;
                AppendChat(String::Format("{0}: {1}", player, message));
            })));
        }

        void Canvas_MouseClick(Object^ sender, MouseEventArgs^ e) {
            auto cell = _renderer->PointToCell(e->Location);
            if (cell == nullptr || _board->Winner != nullptr) return;

            if (_net->Mark != _board->NextTurn)
            {
                MessageBox::Show("⏳ Chưa tới lượt của bạn!", "Caro", MessageBoxButtons::OK, MessageBoxIcon::Information);
                return;
            }

            _net->SendMove(cell->row, cell->col);
        }

        void UpdateBoard() {
            _lblTurn->Text = String::Format("NextTurn: {0}", _board->NextTurn);
            if (_board->Winner != nullptr)
                _lblTurn->Text += String::Format("  |  Winner: {0}", _board->Winner);
            _renderer->Refresh();
        }

        void ShowWinningLine(List<Point>^ winningCells) {
            _renderer->WinningCells = winningCells;
            _renderer->Refresh();
        }

        void AppendChat(String^ line) {
            if (txtChat->InvokeRequired)
                txtChat->Invoke(gcnew Action(gcnew Action([this, line]() { AppendChat(line); })));
            else
                txtChat->AppendText(line + Environment::NewLine);
        }

        void FormMain_Load(Object^ sender, EventArgs^ e) {}
    };
}}
