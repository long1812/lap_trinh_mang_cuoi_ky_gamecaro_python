#include "Network/Program.h"
#include "Network/ServerHost.h"
using namespace ServerApp;
using namespace Server;
using namespace System;

void Program::Main(array<String^>^ args)
{
    ServerHost^ server = gcnew ServerHost(9000);
    server->Start();
    Console::WriteLine("Server is running on port 9000...");
    Console::ReadLine();
    server->Stop();
}
