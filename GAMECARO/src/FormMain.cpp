#include "Forms/FormMain.h"
#include "Network/ConnectToServer.h"
#include "Game/GameRenderer.h"
#include "Game/GameLoop.h"
#include "Game/Board.h"
using namespace Client::Forms;
using namespace Client::Game;
using namespace Client::Network;
using namespace System;
using namespace System::Windows::Forms;
using namespace System::Drawing;
using namespace System::Collections::Generic;

FormMain::FormMain(String^ playerName)
{
    _playerName = playerName;
    _board = gcnew Board();
    InitializeComponent();

    // ==== UI Panels + Labels + Canvas ====
    // (giống code C# ban đầu, mình giữ nguyên)
    // … thêm ở đây _canvas, _lblTurn, _lblMark, _lblTimer, _lblScore, _lblWins, _lblLosses …

    _renderer = gcnew GameRenderer(_canvas, _board);
    _loop = gcnew GameLoop(this);
    _loop->Start();

    _net = gcnew ConnectToServer("127.0.0.1", 9000, _board);

    // Event binding
    _net->OnBoardChanged += gcnew Action(this, &FormMain::OnBoardChanged);
    _net->OnWinner += gcnew Action<String^>(this, &FormMain::OnWinner);
    _net->OnInitReceived += gcnew Action<String^>(this, &FormMain::OnInitReceived);
    _net->OnTimerUpdate += gcnew Action<int, String^>(this, &FormMain::OnTimerUpdate);
    _net->OnReset += gcnew Action(this, &FormMain::OnReset);
    _net->OnRankUpdate += gcnew Action<int,int,int>(this, &FormMain::OnRankUpdate);
    _net->OnChatReceived += gcnew Action<String^, String^>(this, &FormMain::OnChatReceived);

    _net->ConnectAndJoin(_playerName);

    // Mouse click handler
    _canvas->MouseClick += gcnew MouseEventHandler(this, &FormMain::Canvas_MouseClick);
}

void FormMain::OnFormClosed(FormClosedEventArgs^ e)
{
    _loop->Stop();
    _net->Disconnect();
    Form::OnFormClosed(e);
}
