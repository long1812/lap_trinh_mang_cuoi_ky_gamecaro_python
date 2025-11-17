#pragma once
#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include "GameManager.h"
using namespace System;
using namespace System::Net::Sockets;
using namespace System::Threading;

namespace Server {

    public ref class ClientHandler
    {
    private:
        TcpClient^ client;
        GameManager^ manager;
        Thread^ clientThread;

    public:
        ClientHandler(TcpClient^ c, GameManager^ gm);
        void Start();
        void HandleClient();
        void SendMessage(String^ msg);
    };

}

#endif
