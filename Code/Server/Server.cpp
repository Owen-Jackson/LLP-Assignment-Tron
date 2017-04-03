#include "Server.h"
#include <Game\SharedData.h>
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

sf::Packet& operator << (sf::Packet& packet, sf::Color colour)
{
	packet << colour.r << colour.g << colour.b << colour.a;
	return packet;
}

sf::Packet& operator << (sf::Packet& packet, sf::Vector2f& pos)
{
	packet << pos.x << pos.y;
	return packet;
}

Server::Server()
{
	;
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
		std::cout << player_number << std::endl;

		//add controls
		init_pack << start_data.up[player_number % 4] << start_data.left[player_number % 4] << start_data.down[player_number % 4] << start_data.right[player_number % 4];
		//std::cout << player_number % 4 << std::endl;
		//add player color
		init_pack << start_data.colours[player_number % 4];

		//add start position
		init_pack << start_data.starting_positions[player_number % 4];

		//send pack to new client
		client.getSocket().send(init_pack);

		//add client to the current list
		client.setPosition(start_data.starting_positions[player_number % 4].x, start_data.starting_positions[player_number % 4].y);
		client.setSpawn(start_data.starting_positions[player_number % 4].x, start_data.starting_positions[player_number % 4].y);
		//std::cout << "player " << client.getClientID() << " position pre move: " << client.getPosition().x << " , " << client.getPosition().y << std::endl;
		tcp_clients.push_back(std::move(client));
		std::cout << "client " << client.getClientID() << " connected" << std::endl;

		//set new client's position
		//tcp_clients[player_number].setPosition(start_data.starting_positions[player_number % 4].x, start_data.starting_positions[player_number % 4].y);
		//std::cout << "player " << tcp_clients[0].getClientID() << " position: " << tcp_clients[0].getPosition().x << " , " << tcp_clients[0].getPosition().y << std::endl;
		//increment the player number
		player_number++;
		std::cout << "number of players: " << player_number << std::endl;

		//Tell existing players that a new player has joined
		sf::Packet rival_appears;
		rival_appears << NetMsg::NEWPLAYER;
		rival_appears << client.getClientID();
		for (auto& iter : tcp_clients)
		{
			if (iter == client)
			{
				continue;
			}
			iter.getSocket().send(rival_appears);
		}
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
				//std::cout << "received player position" << std::endl;
				//get move direction from client as int
				sf::Uint32 new_dir;
				packet >> new_dir;
				//cast direction to PlayerMove enum then store the value
				PlayerMove direction = static_cast<PlayerMove>(new_dir);
				iter->setDirection(direction);
				std::cout << "player " << iter->getClientID() << " moving: " << new_dir << std::endl;
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
				mutex.lock();
				for (auto& client : tcp_clients)
				{
					//tick each client, updating their position and sprite position
					client.Tick();
				}
				updatePositions(tcp_clients);
				mutex.unlock();
				clock.restart();
			}
		}
	}
}

//Send each client their own position and other player's positions
void Server::updatePositions(TcpClients& tcp_clients)
{
	sf::Packet packet;
	packet << NetMsg::GRID_STATE;
	for (auto& client : tcp_clients)
	{
		packet << client.getPosition().x << client.getPosition().y;
	}
	for (auto& client : tcp_clients)
	{
		client.getSocket().send(packet);
	}
}