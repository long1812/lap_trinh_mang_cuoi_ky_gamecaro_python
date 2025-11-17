#include "Network/ConnectToServer.h"
#include "Game/Board.h"
using namespace Client::Network;
using namespace Client::Game;

ConnectToServer::ConnectToServer(String^ h, int p, Board^ b)
{
    host = h;
    port = p;
    board = b;
    Mark = "";
    client = nullptr;
    stream = nullptr;
}

void ConnectToServer::ConnectAndJoin(String^ playerName)
{
    try
    {
        client = gcnew TcpClient(host, port);
        stream = client->GetStream();

        // Send join message (simple protocol, e.g., "JOIN:playerName")
        array<Byte>^ data = Encoding::UTF8->GetBytes("JOIN:" + playerName);
        stream->Write(data, 0, data->Length);

        // Start listening thread
        listenThread = gcnew Thread(gcnew ThreadStart(this, &ConnectToServer::ListenServer));
        listenThread->IsBackground = true;
        listenThread->Start();
    }
    catch(Exception^ e)
    {
        Console::WriteLine("Connect error: {0}", e->Message);
    }
}

void ConnectToServer::ListenServer()
{
    array<Byte>^ buffer = gcnew array<Byte>(1024);
    int bytesRead;

    try
    {
        while(client->Connected)
        {
            bytesRead = stream->Read(buffer, 0, buffer->Length);
            if(bytesRead > 0)
            {
                String^ msg = Encoding::UTF8->GetString(buffer, 0, bytesRead);
                ProcessMessage(msg);
            }
        }
    }
    catch(Exception^ e)
    {
        Console::WriteLine("Server disconnected: {0}", e->Message);
        Disconnect();
    }
}

void ConnectToServer::ProcessMessage(String^ msg)
{
    // Very simple protocol parsing (for demo)
    // Example: "BOARD:X:row:col", "WINNER:X", "TIMER:sec:turn", "RESET", "RANK:score:wins:losses", "CHAT:player:message"
    array<String^>^ parts = msg->Split(':');
    if(parts->Length == 0) return;

    String^ cmd = parts[0];

    if(cmd == "BOARD" && parts->Length == 4)
    {
        String^ mark = parts[1];
        int row = Int32::Parse(parts[2]);
        int col = Int32::Parse(parts[3]);

        board->PlaceMove(row, col, mark);

        if(OnBoardChanged != nullptr) OnBoardChanged();
    }
    else if(cmd == "WINNER" && parts->Length == 2)
    {
        if(OnWinner != nullptr) OnWinner(parts[1]);
    }
    else if(cmd == "INIT" && parts->Length == 2)
    {
        Mark = parts[1];
        if(OnInitReceived != nullptr) OnInitReceived(Mark);
    }
    else if(cmd == "TIMER" && parts->Length == 3)
    {
        int sec = Int32::Parse(parts[1]);
        int turn = Int32::Parse(parts[2]);
        if(OnTimerUpdate != nullptr) OnTimerUpdate(sec, turn);
    }
    else if(cmd == "RESET")
    {
        if(OnReset != nullptr) OnReset();
    }
    else if(cmd == "RANK" && parts->Length == 4)
    {
        int score = Int32::Parse(parts[1]);
        int wins = Int32::Parse(parts[2]);
        int losses = Int32::Parse(parts[3]);
        if(OnRankUpdate != nullptr) OnRankUpdate(score, wins, losses);
    }
    else if(cmd == "CHAT" && parts->Length >= 3)
    {
        String^ player = parts[1];
        String^ message = String::Join(":", parts, 2, parts->Length - 2);
        if(OnChatReceived != nullptr) OnChatReceived(player, message);
    }
}

void ConnectToServer::SendMove(int row, int col)
{
    if(client == nullptr || !client->Connected) return;

    String^ msg = String::Format("MOVE:{0}:{1}", row, col);
    array<Byte>^ data = Encoding::UTF8->GetBytes(msg);
    stream->Write(data, 0, data->Length);
}

void ConnectToServer::SendChat(String^ message)
{
    if(client == nullptr || !client->Connected) return;

    String^ msg = "CHAT:" + message;
    array<Byte>^ data = Encoding::UTF8->GetBytes(msg);
    stream->Write(data, 0, data->Length);
}

void ConnectToServer::SendSurrender()
{
    if(client == nullptr || !client->Connected) return;

    String^ msg = "SURRENDER";
    array<Byte>^ data = Encoding::UTF8->GetBytes(msg);
    stream->Write(data, 0, data->Length);
}

void ConnectToServer::Disconnect()
{
    try
    {
        if(stream != nullptr) stream->Close();
        if(client != nullptr) client->Close();
        if(listenThread != nullptr && listenThread->IsAlive) listenThread->Abort();
    }
    catch(...) {}
}
