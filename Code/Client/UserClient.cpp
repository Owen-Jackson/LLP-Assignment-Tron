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

UserClient::UserClient()
{
	player = std::make_unique<Player>();
}

bool UserClient::connect(TcpClient& client)
{
	//attempt to connect to server
	auto status = client.connect(SERVER_IP, SERVER_TCP_PORT);
	if (status != sf::Socket::Done)
	{
		player->getSprite()->setFillColor(sf::Color::Blue);
		return false;
	}

	//successful connect
	player->getSprite()->setFillColor(sf::Color::Red);
	client.setBlocking(false);

	return true;
}

void UserClient::input(TcpClient& client)
{
	while (true)
	{
		sf::Packet packet;
		//std::string input;
		//std::getline(std::cin, input);
		packet << NetMsg::PLAYER << player.get();
		client.send(packet);
	}
}

void UserClient::client()
{
	TcpClient socket;
	while(!connect(socket))
	{
		;
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

	return input(socket);
}

const void UserClient::addOpponent(Player* opponent)
{
	enemies.resize(enemies.size() + 1);
	enemies.insert(enemies.end(), opponent);
}