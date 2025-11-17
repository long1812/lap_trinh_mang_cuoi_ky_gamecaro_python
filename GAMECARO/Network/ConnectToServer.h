#pragma once
#ifndef CONNECTTOSERVER_H
#define CONNECTTOSERVER_H

#include "Board.h"
#include <functional>

using namespace System;
using namespace System::Net::Sockets;
using namespace System::Threading;
using namespace System::Text;

namespace Client { namespace Network {

    public ref class ConnectToServer
    {
    private:
        String^ host;
        int port;
        TcpClient^ client;
        NetworkStream^ stream;
        Thread^ listenThread;

        Client::Game::Board^ board;

    public:
        // Player mark (X/O)
        String^ Mark;

        // Events / Callbacks
        Action^ OnBoardChanged;
        Action<String^>^ OnWinner;
        Action<String^>^ OnInitReceived;
        Action<int,int>^ OnTimerUpdate; // sec, turn (or you can change to proper struct)
        Action^ OnReset;
        Action<int,int,int>^ OnRankUpdate; // score, wins, losses
        Action<String^, String^>^ OnChatReceived; // player, message

        ConnectToServer(String^ host, int port, Client::Game::Board^ board);

        void ConnectAndJoin(String^ playerName);
        void SendMove(int row, int col);
        void SendChat(String^ message);
        void SendSurrender();
        void Disconnect();

    private:
        void ListenServer();
        void ProcessMessage(String^ msg);
    };

}}
#endif
