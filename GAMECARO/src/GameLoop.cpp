#include "Game/GameLoop.h"
using namespace Client::Game;
using namespace System::Threading;

GameLoop::GameLoop(Form^ mainForm)
{
    _form = mainForm;
    _running = false;
}

void GameLoop::Start()
{
    _running = true;
    _thread = gcnew Thread(gcnew ThreadStart(this, &GameLoop::Loop));
    _thread->IsBackground = true;
    _thread->Start();
}

void GameLoop::Stop()
{
    _running = false;
}

void GameLoop::Loop()
{
    while(_running)
    {
        Thread::Sleep(50);
        _form->Invalidate();
    }
}
