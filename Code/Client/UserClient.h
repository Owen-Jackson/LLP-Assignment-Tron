#pragma once
#include <SFML/Network.hpp>
#include <memory>
#include <vector>

using TcpClient = sf::TcpSocket;
using TcpClientPtr = std::unique_ptr<TcpClient>;
using TcpClients = std::vector<TcpClient>;

class UserClient
{
public:
	UserClient() = default;
	~UserClient() = default;

	bool connect(TcpClient&);
	void input(TcpClient&);
	void client();
};