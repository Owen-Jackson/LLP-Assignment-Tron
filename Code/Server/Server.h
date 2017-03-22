#pragma once
#include <SFML/Network.hpp>
#include <memory>
#include <vector>
#include <chrono>
#include "Client.h"

using TcpClient = sf::TcpSocket;
using TcpClientPtr = std::unique_ptr<TcpClient>;
using TcpClients = std::vector<Client>;

class Server
{
public:
	Server() = default;
	~Server() = default;
	bool bindServerPort(sf::TcpListener&);
	void connect(sf::TcpListener&, sf::SocketSelector&, TcpClients&);
	void receiveMsg(TcpClients&, sf::SocketSelector&);
	void listen(sf::TcpListener&, sf::SocketSelector&, TcpClients&);
	void runMe();
	void ping(TcpClients&);
};