#include "Network/sServerHost.h"

using namespace Server;
using namespace System;
using namespace System::Net;
using namespace System::Net::Sockets;
using namespace System::Threading;
using namespace System::Collections::Generic;

ServerHost::ServerHost(int port)
{
    gm = gcnew GameManager();
    clients = gcnew List<ClientHandler^>();
    listener = gcnew TcpListener(IPAddress::Any, port);
    running = false;
}

void ServerHost::Start()
{
    running = true;
    listener->Start();
    Thread^ acceptThread = gcnew Thread(gcnew ThreadStart(this, &ServerHost::AcceptClients));
    acceptThread->IsBackground = true;
    acceptThread->Start();
    Console::WriteLine("Server started...");
}

void ServerHost::Stop()
{
    running = false;
    listener->Stop();
    Console::WriteLine("Server stopped.");
}

void ServerHost::AcceptClients()
{
    while(running)
    {
        try
        {
            TcpClient^ client = listener->AcceptTcpClient();
            ClientHandler^ handler = gcnew ClientHandler(client, gm);
            clients->Add(handler);
            handler->Start();
            Console::WriteLine("New client connected.");
        }
        catch(Exception^ e)
        {
            Console::WriteLine("Accept client error: {0}", e->Message);
        }
    }
}
