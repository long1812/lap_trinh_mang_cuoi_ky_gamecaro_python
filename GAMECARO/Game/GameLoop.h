#pragma once
#include "FormMain.h"

using namespace System;
using namespace System::Threading;

namespace Client {
namespace Game {

    public ref class GameLoop
    {
    public:
        GameLoop(Form^ mainForm)
        {
            _form = mainForm;
            _running = false;
        }

        void Start()
        {
            _running = true;
            _thread = gcnew Thread(gcnew ThreadStart(this, &GameLoop::Loop));
            _thread->IsBackground = true;
            _thread->Start();
        }

        void Stop()
        {
            _running = false;
        }

    private:
        Form^ _form;
        Thread^ _thread;
        bool _running;

        void Loop()
        {
            while (_running)
            {
                Thread::Sleep(50);
                _form->Invalidate();
            }
        }
    };
}}
