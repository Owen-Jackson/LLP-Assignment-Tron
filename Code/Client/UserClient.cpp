#include "UserClient.h"
#include <Game/SharedData.h>

#include <future>
#include <iostream>
#include <string>

using TcpClient = sf::TcpSocket;
using TcpClientPtr = std::unique_ptr<TcpClient>;
using TcpClients = std::vector<TcpClient>;
using Key = sf::Keyboard::Key;

const int SERVER_TCP_PORT = 53000;
const sf::IpAddress SERVER_IP = "127.0.0.1";
int CURRENT_PORT = SERVER_TCP_PORT;

sf::Packet& operator >> (sf::Packet& packet, InitData& controls)
{
	packet >> controls.up >> controls.left >> controls.down >> controls.right;
	return packet;
}

sf::Packet& operator << (sf::Packet& packet, const PlayerMove& dir)
{
	packet << static_cast<sf::Uint32>(dir);
	return packet;
}

UserClient::UserClient()
{
	my_player.sprite = std::make_unique<sf::RectangleShape>();
	my_player.sprite->setSize(sf::Vector2f((float)WindowSize::width / WindowSize::grid_size, (float)WindowSize::height / WindowSize::grid_size));
	my_player.sprite->setFillColor(sf::Color::White);
	my_player.move_dir = PlayerMove::NONE;
	my_player.xPos = 0;
	my_player.yPos = 0;
	players.push_back(std::move(my_player));
}

bool UserClient::connect(TcpClient& client)
{
	//attempt to connect to server
	auto status = client.connect(SERVER_IP, SERVER_TCP_PORT);
	if (status != sf::Socket::Done)
	{
		players[0].sprite->setFillColor(sf::Color::Blue);
		return false;
	}

	//successful connect
	players[0].sprite->setFillColor(sf::Color::Red);
	client.setBlocking(false);

	return true;
}

void UserClient::input(TcpClient& client)
{
	sf::Keyboard keyboard;
	while (true)
	{
		//Check for keyboard input to move the player
		if (keyboard.isKeyPressed((Key)controls.up))
		{
			sf::Packet packet;
			packet << NetMsg::MOVEDIR << PlayerMove::UP;
			client.send(packet);
		}
		else if (keyboard.isKeyPressed((Key)controls.left))
		{
			sf::Packet packet;
			packet << NetMsg::MOVEDIR << LEFT;
			client.send(packet);
		}
		else if (keyboard.isKeyPressed((Key)controls.down))
		{
			sf::Packet packet;
			packet << NetMsg::MOVEDIR << DOWN;
			client.send(packet);
		}
		else if (keyboard.isKeyPressed((Key)controls.right))
		{
			sf::Packet packet;
			packet << NetMsg::MOVEDIR << RIGHT;
			client.send(packet);
		}
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
				else if (msg == NetMsg::INIT)
				{
					players[0].sprite->setFillColor(sf::Color::Yellow);
					InitData scheme;
					packet >> scheme;
					setControls(scheme);
				}
				else if (msg == NetMsg::PING)
				{
					sf::Packet packet;
					packet << NetMsg::PONG;
					socket.send(packet);
				}
				else if (msg == NetMsg::POSITIONS)
				{
					//mutex.lock();
					packet >> players[0].xPos >> players[0].yPos;
					players[0].sprite->setPosition(players[0].xPos, players[0].yPos);
					//opponent->setPosition(opp_pos);
					//mutex.unlock();
				}
			}
		} while (status != sf::Socket::Disconnected);
	});

	return input(socket);
}
//
//const void UserClient::addOpponent(std::unique_ptr<Player> opponent)
//{
//	enemies.push_back(std::move(opponent));
//}

void UserClient::setControls(InitData& new_controls)
{
	controls.up = new_controls.up;
	controls.left = new_controls.left;
	controls.down = new_controls.down;
	controls.right = new_controls.right;
}