#include "UserClient.h"
#include <Game/SharedData.h>

#include <future>
#include <iostream>
#include <string>

using TcpClient = sf::TcpSocket;
using TcpClientPtr = std::unique_ptr<TcpClient>;
using TcpClients = std::vector<TcpClient>;

const int SERVER_TCP_PORT = 53000;
const sf::IpAddress SERVER_IP = "127.0.0.1";
int CURRENT_PORT = SERVER_TCP_PORT;

bool UserClient::connect(TcpClient& client)
{
	//attempt to connect to server
	auto status = client.connect(SERVER_IP, SERVER_TCP_PORT);
	if (status != sf::Socket::Done)
	{
		std::cout << "could not connect" << std::endl;
		return false;
	}


	//successful connect
	std::cout << "Connected to server: " << SERVER_IP;
	std::cout << std::endl;

	return true;
}

void UserClient::input(TcpClient& client)
{
	while (true)
	{
		sf::Packet packet;
		packet << NetMsg::PLAYER << player.get();
		client.send(packet);
	}
}

void UserClient::client()
{
	TcpClient socket;
	if (!connect(socket))
	{
		return;
	}

	auto handle = std::async(std::launch::async, [&]
	{
		//keep track of socket status
		sf::Socket::Status status;

		do
		{
			sf::Packet packet;
			status = socket.receive(packet);
			if (status == sf::Socket::Done)
			{
				int header = 0;
				packet >> header;

				NetMsg msg = static_cast<NetMsg>(header);
				if (msg == NetMsg::CHAT)
				{
					std::string message;
					packet >> message;
					std::cout << "message is: " << message << std::endl;
				}
				else if (msg == NetMsg::PING)
				{
					sf::Packet packet;
					packet << NetMsg::PONG;
					socket.send(packet);
				}
			}
		} while (status != sf::Socket::Disconnected);
	});
	//std::cout << "disconnected";
	return input(socket);
}