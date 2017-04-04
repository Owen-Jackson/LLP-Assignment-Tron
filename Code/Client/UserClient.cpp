#include "UserClient.h"

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

sf::Packet& operator >> (sf::Packet& packet, Controls& controls)
{
	packet >> controls.up >> controls.left >> controls.down >> controls.right;
	return packet;
}

sf::Packet& operator >> (sf::Packet& packet, sf::Vector2i& pos)
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
	//set to be alive
	my_data.is_alive = true;

	//setup the grid to be unused values
	for (int i = 0; i < WindowSize::grid_size; i++)
	{
		for (int j = 0; j < WindowSize::grid_size; j++)
		{
			grid.push_back(-1);
		}
	}
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
	PlayerMove last_dir = my_data.move_dir;
	sf::Packet dir_pack;
	while (true)
	{
		//Check for keyboard input to move the player
		if (keyboard.isKeyPressed((Key)my_data.controls.up) && last_dir != DOWN)
		{
			my_data.move_dir = UP;
			if (my_data.move_dir != last_dir)
			{
				dir_pack << NetMsg::MOVEDIR << PlayerMove::UP;
				client.send(dir_pack);
			}
		}
		else if (keyboard.isKeyPressed((Key)my_data.controls.left) && last_dir != RIGHT)
		{
			my_data.move_dir = LEFT;
			if (my_data.move_dir != last_dir)
			{
				dir_pack << NetMsg::MOVEDIR << PlayerMove::LEFT;
				client.send(dir_pack);
			}
		}
		else if (keyboard.isKeyPressed((Key)my_data.controls.down) && last_dir != UP)
		{
			my_data.move_dir = DOWN;
			if (my_data.move_dir != last_dir)
			{
				dir_pack << NetMsg::MOVEDIR << PlayerMove::DOWN;
				client.send(dir_pack);
			}
		}
		else if (keyboard.isKeyPressed((Key)my_data.controls.right) && last_dir != LEFT)
		{
			my_data.move_dir = RIGHT;
			if (my_data.move_dir != last_dir)
			{
				dir_pack << NetMsg::MOVEDIR << PlayerMove::RIGHT;
				client.send(dir_pack);
			}
		}
		last_dir = my_data.move_dir;
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
					Controls scheme;
					if (packet >> scheme)
					{
						setControls(scheme);
					}
					sf::Uint32 move;
					packet >> move;
					my_data.move_dir = (PlayerMove)move;
					my_data.start_dir = (PlayerMove)move;

				}
				else if (msg == NetMsg::PING)
				{
					sf::Packet packet;
					packet << NetMsg::PONG;
					socket.send(packet);
				}
				else if (msg == NetMsg::GRID_STATE)
				{
					mutex.lock();
					sf::Uint32 size;
					packet >> size;
					for (int i = 0; i < size; i++)
					{
						int index;
						packet >> index;
						grid[i] = index;
					}
					mutex.unlock();
				}
				else if (msg == NetMsg::RESET)
				{
					my_data.move_dir = my_data.start_dir;
				}
			}
		} while (status != sf::Socket::Disconnected);
	});

	return input(socket);
}

void UserClient::setControls(Controls& new_controls)
{
	my_data.controls.up = new_controls.up;
	my_data.controls.left = new_controls.left;
	my_data.controls.down = new_controls.down;
	my_data.controls.right = new_controls.right;
}