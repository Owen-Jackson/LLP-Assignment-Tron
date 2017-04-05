//System libs
#include <iostream>
#include <string>
#include <algorithm>
#include <utility>
#include <future>
#include <thread>

//SFML
#include <Game\SharedData.h>

//User defined
#include "Server.h"

using TcpClient = sf::TcpSocket;
using TcpClientPtr = std::unique_ptr<TcpClient>;
using TcpClients = std::vector<Client>;

const int SERVER_TCP_PORT = 53000;
const sf::IpAddress SERVER_IP = "127.0.0.1";
int CURRENT_PORT = SERVER_TCP_PORT;

//custom operators
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

		//make the client equal to the first in the missing client id queue (if needed)
		if (disconnected_player_ids.size() > 0)
		{
			next_id = disconnected_player_ids.top();
			disconnected_player_ids.pop();
		}
		auto& client = Client(client_ptr, next_id);

		//give client their control scheme
		mutex.lock();
		sf::Packet init_pack;
		init_pack << NetMsg::INIT;

		//add client ID
		init_pack << (sf::Uint8)client.getClientID();

		//add controls
		init_pack << start_data.up[next_id % 4] << start_data.left[next_id % 4] << start_data.down[next_id % 4] << start_data.right[next_id % 4];
	
		//add start direction
		init_pack << start_data.start_dir[next_id % 4];
		client.setStartDirection(start_data.start_dir[next_id % 4]);

		//send pack to new client
		client.getSocket().send(init_pack);

		//set client's initial position and respawn point
		client.setSpawn(start_data.starting_positions[next_id % 4]);

		//add client to the client list
		tcp_clients.push_back(std::move(client));
		std::cout << "client " << client.getClientID() << " connected" << std::endl;

		//make next_id = player_number + 1 if missing id queue is empty
		if (disconnected_player_ids.size() == 0)
		{
			next_id = player_number + 1;
		}

		//increment the player number
		player_number++;
		std::cout << "number of players: " << player_number << std::endl;

		//refresh grid for the current players
		refreshGrid(tcp_clients);
		mutex.unlock();
	}
}

void Server::disconnect(TcpClients& tcp_clients, Client& client, sf::SocketSelector& selector)
{
	mutex.lock();
	sf::Socket& socket = client.getSocket();
	//remove socket from selector and disconnect
	selector.remove(socket);
	client.getSocket().disconnect();
	std::cout << "disconnect occurred" << std::endl;
	std::cout << "Client " << client.getClientID() << " disconnected" << std::endl;

	//add client's id to the disconnected queue
	disconnected_player_ids.push(client.getClientID());

	//remove-erase the client
	tcp_clients.erase(std::remove(tcp_clients.begin(), tcp_clients.end(), client), tcp_clients.end());

	//decrement total players
	player_number--;
	mutex.unlock();
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
			//check if client has disconnected
			if (sender_ref.receive(packet) == sf::Socket::Disconnected)
			{
				disconnect(tcp_clients, *iter, selector);
				break;
			}
			//read the message header
			int header = 0;
			packet >> header;

			NetMsg msg = static_cast<NetMsg>(header);

			if (msg == NetMsg::MOVEDIR)
			{
				//get move direction from client as int
				mutex.lock();
				sf::Uint32 new_dir;
				packet >> new_dir;

				//cast direction to PlayerMove enum then store the value
				PlayerMove direction = static_cast<PlayerMove>(new_dir);
				iter->setDirection(direction);
				mutex.unlock();
			}
			else if (msg == NetMsg::PONG)
			{
				//get the client's latency
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
		if (selector.wait(sf::seconds(10)))
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

	//thread for the game logic
	auto handle1 = std::async(std::launch::async, [&]
	{
		runGame(tcp_clients); 
	});

	//thread to constantly update the grid  and sent it to the clients
	auto handle2 = std::async(std::launch::async, [&]
	{
		updateGrid(tcp_clients);
	});

	return listen(tcp_listener, selector, tcp_clients);
}

void Server::ping(TcpClients& tcp_clients)
{
	//loop through all clients and ping them
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
				int alive_players = player_number;
				mutex.lock();
				for (auto& client : tcp_clients)
				{
					//tick each alive client, updating their position and sprite position
					if (client.isAlive())
					{
						client.tick();
						//check collisions and kill the player if true
						if (grid[(client.getIndexPosition().x * WindowSize::grid_size) + client.getIndexPosition().y] != -1)
						{
							killPlayer(client);
						}
					}
					else
					{
						alive_players--;
					}
				}
				//reset grid when there are multiple players
				if (alive_players <= 1 && player_number > 1)
				{
					refreshGrid(tcp_clients);
					alive_players = player_number;
				}
				//reset the grid for when 1 player is connected to the server
				else if (alive_players == 0)
				{
					refreshGrid(tcp_clients);
					alive_players = player_number;
				}
				mutex.unlock();
				clock.restart();
			}
		}
	}
}

//Send each client the current state of the grid
void Server::sendPositions(TcpClients& tcp_clients)
{
	sf::Packet packet;
	packet << NetMsg::GRID_STATE;
	packet << grid;
	for (auto& client : tcp_clients)
	{
		client.getSocket().send(packet);
	}
}

void Server::updateGrid(TcpClients& tcp_clients)
{
	while (true)
	{
		mutex.lock();
		for (int i = 0; i < WindowSize::grid_size; i++)
		{
			for (int j = 0; j < WindowSize::grid_size; j++)
			{
				for (auto& client : tcp_clients)
				{
					if (client.isAlive())
					{
						//set the grid id at the client's index to be equal to their id
						if (sf::Vector2i(j, i) == client.getIndexPosition())
						{
							grid[(j * WindowSize::grid_size) + i] = client.getClientID();
						}
					}
				}
			}
		}
		mutex.unlock();
		sendPositions(tcp_clients);
	}
}

//Resets the entire grid; used when one player is left in a multiplayer game (or when the only connected player dies)
void Server::refreshGrid(TcpClients& tcp_clients)
{
	mutex.lock();
	for (auto& id : grid)
	{
		id = -1;
	}
	for (auto& client : tcp_clients)
	{
		client.respawn();
	}
	mutex.unlock();
}

//remove the player's trail from the grid and set them to be dead
void Server::killPlayer(Client& client)
{
	mutex.lock();
	for (auto& id : grid)
	{
		if (id == client.getClientID())
		{
			id = -1;
		}
	}
	client.killMe();
	mutex.unlock();
}