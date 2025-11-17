#include "Network/ClientHandler.h"
#include "Game/GameManager.h"
using namespace Server;
using namespace System;
using namespace System::Net::Sockets;
using namespace System::Text;
using namespace System::Threading;

ClientHandler::ClientHandler(TcpClient^ c, GameManager^ gm)
{
    client = c;
    manager = gm;
}

void ClientHandler::Start()
{
    clientThread = gcnew Thread(gcnew ThreadStart(this, &ClientHandler::HandleClient));
    clientThread->IsBackground = true;
    clientThread->Start();
}

void ClientHandler::HandleClient()
{
    try
    {
        NetworkStream^ stream = client->GetStream();
        array<Byte>^ buffer = gcnew array<Byte>(1024);
        int bytesRead;

        while((bytesRead = stream->Read(buffer, 0, buffer->Length)) != 0)
        {
            String^ message = Encoding::UTF8->GetString(buffer, 0, bytesRead);
            // TODO: xử lý message từ client
        }
    }
    catch(Exception^ e)
    {
        Console::WriteLine("Client disconnected: {0}", e->Message);
    }
    finally
    {
        client->Close();
    }
}

void ClientHandler::SendMessage(String^ msg)
{
    try
    {
        array<Byte>^ data = Encoding::UTF8->GetBytes(msg);
        client->GetStream()->Write(data, 0, data->Length);
    }
    catch(Exception^ e)
    {
        Console::WriteLine("Send message error: {0}", e->Message);
    }
}
