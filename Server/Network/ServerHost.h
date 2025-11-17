#pragma once
#ifndef SERVERHOST_H
#define SERVERHOST_H

#include "GameManager.h"
#include "ClientHandler.h"
using namespace System;
using namespace System::Net;
using namespace System::Net::Sockets;
using namespace System::Threading;
using namespace System::Collections::Generic;

namespace Server {

    public ref class ServerHost
    {
    private:
        TcpListener^ listener;
        bool running;
        GameManager^ gm;
        List<ClientHandler^>^ clients;

    public:
        ServerHost(int port);
        void Start();
        void Stop();
        void AcceptClients();
    };

}

#endif
