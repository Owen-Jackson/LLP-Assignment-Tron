// TronServerW32.cpp : Defines the entry point for the console application.
//

//System libs
#include <Server/stdafx.h>
#include <iostream>
#include <future>

//User defined
#include "Server.h"

int main()
{
	std::cout << "Welcome to the Tron server. Searching for players...";
	Server server;
	server.runMe();

    return 0;
}