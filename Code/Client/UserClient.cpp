#include "UserClient.h"
#include <Game/SharedData.h>

#include <future>
#include <iostream>
#include <string>
#include <random>

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

sf::Packet& operator >> (sf::Packet& packet, sf::Vector2f& pos)
{
	packet >> pos.x >> pos.y;
	return packet;
}

sf::Packet& operator >> (sf::Packet& packet, sf::Color& colour)
{
	packet >> colour.r >> colour.g >> colour.b >> colour.a;
	return packet;
}

sf::Packet& operator << (sf::Packet& packet, const PlayerMove& dir)
{
	packet << static_cast<sf::Uint32>(dir);
	return packet;
}

UserClient::UserClient()
{
	//my_player.sprite = std::make_unique<sf::RectangleShape>();
	//my_player.sprite->setSize(sf::Vector2f((float)WindowSize::width / WindowSize::grid_size, (float)WindowSize::height / WindowSize::grid_size));
	//my_player.sprite->setFillColor(sf::Color::White);
	//sf::RectangleShape my_sprite;
	//my_sprite.setSize(sf::Vector2f((float)WindowSize::width / WindowSize::grid_size, (float)WindowSize::height / WindowSize::grid_size));
	//player_sprites.push_back(my_sprite);
}

bool UserClient::connect(TcpClient& client)
{
	//attempt to connect to server
	auto status = client.connect(SERVER_IP, SERVER_TCP_PORT);
	if (status != sf::Socket::Done)
	{
		return false;
	}

	//successful connect
	client.setBlocking(false);

	return true;
}

void UserClient::input(TcpClient& client)
{
	sf::Keyboard keyboard;
	sf::Packet dir_pack;
	while (true)
	{
		//Check for keyboard input to move the player
		if (keyboard.isKeyPressed((Key)controls.up))
		{
			dir_pack << NetMsg::MOVEDIR << PlayerMove::UP;
			client.send(dir_pack);
		}
		else if (keyboard.isKeyPressed((Key)controls.left))
		{
			dir_pack << NetMsg::MOVEDIR << LEFT;
			client.send(dir_pack);
		}
		else if (keyboard.isKeyPressed((Key)controls.down))
		{
			dir_pack << NetMsg::MOVEDIR << DOWN;
			client.send(dir_pack);
		}
		else if (keyboard.isKeyPressed((Key)controls.right))
		{
			dir_pack << NetMsg::MOVEDIR << RIGHT;
			client.send(dir_pack);
		}
		dir_pack.clear();
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

					std::unique_ptr<sf::RectangleShape> my_sprite = std::make_unique<sf::RectangleShape>();
					packet >> my_data.client_id;
					if (my_data.client_id > 0)
					{
						createPlayers();
					}
					//receive controls
					InitData scheme;
					if (packet >> scheme)
					{
						setControls(scheme);
					}
					//setControls(scheme);

					my_sprite->setSize(sf::Vector2f((float)WindowSize::width / WindowSize::grid_size, (float)WindowSize::height / WindowSize::grid_size));

					//receive sprite colour
					sf::Color my_colour;
					packet >> my_colour;
					my_sprite->setFillColor(my_colour);

					//receive start position
					sf::Vector2f start_pos;
					packet >> start_pos;
					my_sprite->setPosition(start_pos);

					player_sprites.push_back(std::move(my_sprite));
				}
				else if (msg == NetMsg::PING)
				{
					sf::Packet packet;
					packet << NetMsg::PONG;
					socket.send(packet);
				}
				else if (msg == NetMsg::POSITIONS)
				{
					sf::Vector2f pos;
					for (int i = 0; i < player_sprites.size(); i++)
					{
						packet >> pos;
						player_sprites[i]->setPosition(pos);
					}
				}
				else if (msg == NetMsg::NEWPLAYER)
				{
					addOpponent(packet);
				}
			}
		} while (status != sf::Socket::Disconnected);
	});

	return input(socket);
}

void UserClient::addOpponent(sf::Packet& packet)
{
	int new_id;
	packet >> new_id;
	std::unique_ptr<sf::RectangleShape> new_player = std::make_unique<sf::RectangleShape>();
	new_player->setSize(sf::Vector2f((float)WindowSize::width / WindowSize::grid_size, (float)WindowSize::height / WindowSize::grid_size));
	new_player->setFillColor(start_data.colours[new_id % 4]);
	new_player->setPosition(start_data.starting_positions[new_id % 4]);
	player_sprites.push_back(std::move(new_player));
}

void UserClient::createPlayers()
{
	for (int i = 0; i < my_data.client_id; i++)
	{
		std::unique_ptr<sf::RectangleShape> sprite = std::make_unique<sf::RectangleShape>();
		sprite->setSize(sf::Vector2f((float)WindowSize::width / WindowSize::grid_size, (float)WindowSize::height / WindowSize::grid_size));
		sprite->setFillColor(start_data.colours[i % 4]);
		sprite->setPosition(start_data.starting_positions[i % 4]);
		player_sprites.push_back(std::move(sprite));
	}
}

void UserClient::setControls(InitData& new_controls)
{
	controls.up = new_controls.up;
	controls.left = new_controls.left;
	controls.down = new_controls.down;
	controls.right = new_controls.right;
}