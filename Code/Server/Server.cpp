#include "Server.h"
#include <Game\SharedData.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <utility>
#include <future>
#include <thread>

using TcpClient = sf::TcpSocket;
using TcpClientPtr = std::unique_ptr<TcpClient>;
using TcpClients = std::vector<Client>;

const int SERVER_TCP_PORT = 53000;
const sf::IpAddress SERVER_IP = "127.0.0.1";
int CURRENT_PORT = SERVER_TCP_PORT;

sf::Packet& operator << (sf::Packet& packet, sf::Color colour)
{
	packet << colour.r << colour.g << colour.b << colour.a;
	return packet;
}

sf::Packet& operator << (sf::Packet& packet, sf::Vector2i& pos)
{
	packet << pos.x << pos.y;
	return packet;
}

sf::Packet& operator << (sf::Packet& packet, std::vector<sf::Int32>& grid)
{
	packet << static_cast<int>(grid.size());
	for (auto& index = grid.begin(); index != grid.end(); index++)
	{
		packet << *index;
	}
	return packet;
}

sf::Packet& operator << (sf::Packet& packet, const PlayerMove& dir)
{
	packet << static_cast<sf::Uint32>(dir);
	return packet;
}

Server::Server()
{
	//setup the grid to be unused values
	for (int i = 0; i < WindowSize::grid_size; i++)
	{
		for (int j = 0; j < WindowSize::grid_size; j++)
		{
			grid.push_back(-1);
		}
	}
}

bool Server::bindServerPort(sf::TcpListener& listener)
{
	if (listener.listen(SERVER_TCP_PORT) != sf::Socket::Done)
	{
		std::cout << "Could not bind server port";
		std::cout << std::endl << "Port: " << SERVER_TCP_PORT;
		std::cout << std::endl;
		return false;
	}
	return true;
}

void Server::connect(sf::TcpListener& tcp_listener, sf::SocketSelector& selector, TcpClients& tcp_clients)
{
	auto client_ptr = new sf::TcpSocket;
	auto& client_ref = *client_ptr;
	//Add new client if socket is available
	if (tcp_listener.accept(client_ref) == sf::Socket::Done)
	{
		selector.add(client_ref);

		auto& client = Client(client_ptr);
		//give client their control scheme
		mutex.lock();
		sf::Packet init_pack;
		init_pack << NetMsg::INIT;

		//add client ID
		init_pack << player_number;
		std::cout << (sf::Uint32)player_number << std::endl;

		//add controls
		init_pack << start_data.up[player_number % 4] << start_data.left[player_number % 4] << start_data.down[player_number % 4] << start_data.right[player_number % 4];
		//std::cout << start_data.up[player_number % 4] << start_data.left[player_number % 4] << start_data.down[player_number % 4] << start_data.right[player_number % 4] << std::endl;

		//add start direction
		init_pack << start_data.start_dir[player_number % 4];
		client.setStartDirection(start_data.start_dir[player_number % 4]);

		//send pack to new client
		client.getSocket().send(init_pack);

		//set client's initial position and respawn point
		client.setSpawn(start_data.starting_positions[player_number % 4]);

		//add client to the client list
		tcp_clients.push_back(std::move(client));
		std::cout << "client " << client.getClientID() << " connected" << std::endl;

		//increment the player number
		player_number++;
		std::cout << "number of players: " << player_number << std::endl;
		mutex.unlock();

		/*if (player_number > 2)
		{
			sf::Packet disconnect_pack;
			disconnect_pack << sf::Socket::Status::Disconnected;
			client.getSocket().send(disconnect_pack);
			std::cout << "not enough room, kicking client " << client.getClientID() << std::endl;
		}*/
	}
}

void Server::disconnect(TcpClients& tcp_clients, Client& client, sf::SocketSelector& selector)
{
	sf::Socket& socket = client.getSocket();
	selector.remove(socket);
	client.getSocket().disconnect();
	std::cout << "disconnect occurred" << std::endl;
	std::cout << "Client " << client.getClientID() << " disconnected" << std::endl;
	std::cout << "number of clients before: " << tcp_clients.size() << std::endl;
	tcp_clients.erase(std::remove(tcp_clients.begin(), tcp_clients.end(), client), tcp_clients.end());
	std::cout << "number of clients after: " << tcp_clients.size() << std::endl;
	player_number--;
}

void Server::receiveMsg(TcpClients& tcp_clients, sf::SocketSelector& selector)
{
	//loop through all tcp clients to find the sender
	for (auto& iter = tcp_clients.begin(); iter != tcp_clients.end(); iter++)
	{
		auto& sender_ref = iter->getSocket();
		if (selector.isReady(sender_ref))
		{
			sf::Packet packet;
			if (sender_ref.receive(packet) == sf::Socket::Disconnected)
			{
				disconnect(tcp_clients, *iter, selector);
				break;
			}
			int header = 0;
			packet >> header;

			NetMsg msg = static_cast<NetMsg>(header);

			if (msg == NetMsg::CHAT)
			{
				std::string string;
				packet >> string;

				//print out the received string
				std::cout << "Net msg: " << string << std::endl;
				std::cout << "Latency: " << iter->getLatency().count()
					<< "us" << std::endl;

				//loop through all connected clients
				for (auto& client = tcp_clients.begin(); client < tcp_clients.end(); client++)
				{
					(*client).getSocket().send(packet);
				}
			}
			else if (msg == NetMsg::MOVEDIR)
			{
				//get move direction from client as int
				//mutex.lock();
				sf::Uint32 new_dir;
				packet >> new_dir;
				//cast direction to PlayerMove enum then store the value
				PlayerMove direction = static_cast<PlayerMove>(new_dir);
				iter->setDirection(direction);
				std::cout << "player " << iter->getClientID() << " moving: " << new_dir << std::endl;
				//mutex.unlock();
			}
			else if (msg == NetMsg::PONG)
			{
				std::cout << "pong called" << std::endl;
				iter->pong();
				std::cout << "Client latency: " << iter->getLatency().count() << "us" << std::endl;
			}
		}
	}
}

void Server::listen(sf::TcpListener& tcp_listener, sf::SocketSelector& selector, TcpClients& tcp_clients)
{
	while (true)
	{
		if (selector.wait(sf::seconds(5)))
		{
			//this is a new conection request
			if (selector.isReady(tcp_listener))
			{
				connect(tcp_listener, selector, tcp_clients);
			}

			//this is a message being receieved
			else
			{
				receiveMsg(tcp_clients, selector);
			}
		}
		else
		{
			ping(tcp_clients);
		}
	}
}

void Server::runMe()
{
	sf::TcpListener tcp_listener;
	if (!bindServerPort(tcp_listener))
	{
		return;
	}

	sf::SocketSelector selector;
	selector.add(tcp_listener);

	TcpClients tcp_clients;
	auto handle1 = std::async(std::launch::async, [&]
	{
		runGame(tcp_clients); 
	});

	return listen(tcp_listener, selector, tcp_clients);
}

void Server::ping(TcpClients& tcp_clients)
{
	//loop through all clients
	constexpr auto timeout = 1s;
	for (auto& client : tcp_clients)
	{
		const auto & timestamp = client.getPingTime();
		const auto now = std::chrono::steady_clock::now();
		auto delta = now - timestamp;
		if (delta > timeout)
		{
			client.ping();
		}
	}
}

//Game loop for updating client positions and collision checks
void Server::runGame(TcpClients& tcp_clients)
{
	sf::Clock clock;
	sf::Time elapsed;
	sf::Time FPS = sf::milliseconds(1000/30);
	while (true)
	{
		while (tcp_clients.size() > 0)
		{
			//Lock FPS
			elapsed = clock.getElapsedTime();
			if (elapsed > FPS)
			{
				updateGrid(tcp_clients);
				for (auto& client : tcp_clients)
				{
					//tick each client, updating their position and sprite position
					client.tick();
					if (grid[(client.getIndexPosition().x * WindowSize::grid_size) + client.getIndexPosition().y] != -1)
					{
						std::cout << "client: " << client.getClientID() << " has died at:" << client.getIndexPosition().x << " , " << client.getIndexPosition().y << std::endl;
						std::cout << "grid index of death: " << grid[(client.getIndexPosition().x * WindowSize::grid_size) + client.getIndexPosition().y] << std::endl;
						refreshGrid(client.getClientID(), tcp_clients);
						client.respawn();
						sf::Packet respawn_pack;
						respawn_pack << NetMsg::RESET;
						client.getSocket().send(respawn_pack);
					}
				}
				clock.restart();
			}
		}
	}
}

//Send each client their own position and other player's positions
void Server::sendPositions(TcpClients& tcp_clients)
{
	sf::Packet packet;
	packet << NetMsg::GRID_STATE;
	packet << grid;
	//for (auto& client : tcp_clients)
	//{
	//	packet << client.getPosition().x << client.getPosition().y;
	//}
	for (auto& client : tcp_clients)
	{
		client.getSocket().send(packet);
	}
}

void Server::updateGrid(TcpClients& tcp_clients)
{
	sf::Packet packet;
	//mutex.lock();
	for (int i = 0; i < WindowSize::grid_size; i++)
	{
		for (int j = 0; j < WindowSize::grid_size; j++)
		{
			mutex.lock();
			for (auto& client : tcp_clients)
			{
				if (sf::Vector2i(j, i) == client.getIndexPosition())
				{
					//mutex.lock();
					grid[(j * WindowSize::grid_size) + i] = client.getClientID();
					//mutex.unlock();
				}
			}
			mutex.unlock();
		}
	}
	sendPositions(tcp_clients);
}

void Server::refreshGrid(const int& client_id, TcpClients& tcp_clients)
{
	for (auto& id : grid)
	{
		if (id == client_id)
		{
			id = -1;
		}
	}
	/*for (auto& client : tcp_clients)
	{
		client.respawn();
	}*/
}