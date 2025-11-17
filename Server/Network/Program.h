#pragma once
#ifndef PROGRAM_H
#define PROGRAM_H

#include "ServerHost.h"
using namespace Server;
using namespace System;

namespace ServerApp {

    ref class Program
    {
    public:
        static void Main(array<String^>^ args);
    };

}

#endif
