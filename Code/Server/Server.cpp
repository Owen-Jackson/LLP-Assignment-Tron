#include "Server.h"
#include <Game/SharedData.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <utility>
#include <future>

using TcpClient = sf::TcpSocket;
using TcpClientPtr = std::unique_ptr<TcpClient>;
using TcpClients = std::vector<Client>;

const int SERVER_TCP_PORT = 53000;
const sf::IpAddress SERVER_IP = "127.0.0.1";
int CURRENT_PORT = SERVER_TCP_PORT;

Server::Server()
{
	//Setup keyboard controls list
	ControlKeys temp;
	controls_set = { temp.player1, temp.player2, temp.player3, temp.player4 };

	//Create grid positions
	std::cout << "generating grid positions" << std::endl;
	for (int i = 0; i < WindowSize::grid_size; i++)
	{
		for (int j = 0; j < WindowSize::grid_size; j++)
		{
			sf::Vector2f position = sf::Vector2f(((float)WindowSize::width / WindowSize::grid_size) * j, ((float)WindowSize::height / WindowSize::grid_size) * i);
			std::cout << position.x << " , " << position.y << std::endl;
			gridPositions.push_back(position);
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
		for (int i = 0; i < 4; i++)
		{
			init_pack << controls_set[player_number % 4][i];
			std::cout << "sending: " << controls_set[player_number % 4][i] << std::endl;
		}
		client.getSocket().send(init_pack);
		//std::cout << "before vector: " << client.getPosition().first << client.getPosition().second << std::endl;
		tcp_clients.push_back(std::move(client));
		std::cout << "client " << client.getClientID() << " connected" << std::endl;
		player_number++;
		tcp_clients[0].setPosition(0, 0);
		std::cout << "number of players: " << player_number << std::endl;
		std::pair<sf::Uint32, sf::Uint32> coords = tcp_clients[0].getPosition();
		//std::cout << "after vector: " << coords.first << " , " << coords.second << std::endl;
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
	//tcp_clients.erase(std::remove(tcp_clients.begin(), tcp_clients.end(), iter),iter);
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
				//std::cout << "received player position" << std::endl;
				//mutex.lock();
				//get move direction from client as int
				sf::Uint32 new_dir;
				packet >> new_dir;
				//cast direction to PlayerMove enum then store the value
				PlayerMove direction = static_cast<PlayerMove>(new_dir);
				if (direction != iter->getMoveDirection())
				{
					std::cout << "player 0 is now moving: " << new_dir << std::endl;
				}
				iter->setDirection(direction);
				//std::cout << "player 0 moving: " << new_dir << std::endl;
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
		if (selector.wait(sf::milliseconds(250)))
		{
			//this is a new conection request
			if (selector.isReady(tcp_listener))
			{
				mutex.lock();
				connect(tcp_listener, selector, tcp_clients);
				mutex.unlock();
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
	auto handle = std::async(std::launch::async, [&]
	{runGame(tcp_clients); });
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
	while (true)
	{
		while (tcp_clients.size() > 0)
		{
			mutex.lock();
			for (auto& client : tcp_clients)
			{
				//tick each client, updating their position and sprite position
				std::cout << client.getPosition().first << " , " << client.getPosition().second << std::endl;
				client.Tick();
			}
			updatePositions(tcp_clients);
			mutex.unlock();
		}
	}
}

//Send each client their own position and other player's positions
void Server::updatePositions(TcpClients& tcp_clients)
{
	sf::Packet packet;
	packet << NetMsg::POSITIONS;
	for (auto& client : tcp_clients)
	{
		packet << client.getPosition().first << client.getPosition().second;
		//std::cout << client.getPosition().first << " , " << client.getPosition().second << std::endl;
	}
	for (auto& client : tcp_clients)
	{
		client.getSocket().send(packet);
	}
}