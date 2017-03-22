// TronServerW32.cpp : Defines the entry point for the console application.
//

#include <Server/stdafx.h>
#include <iostream>
#include "Server.h"

unsigned int Client::next_id = 0;

int main()
{
	std::cout << "Searching for life signs...";
	Server server;
	server.runMe();

    return 0;
}

