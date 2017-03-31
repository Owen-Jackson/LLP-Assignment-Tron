#include "Server.h"
#include <Game/SharedData.h>
#include <iostream>
#include <string>
#include <algorithm>

using TcpClient = sf::TcpSocket;
using TcpClientPtr = std::unique_ptr<TcpClient>;
using TcpClients = std::vector<Client>;

const int SERVER_TCP_PORT = 53000;
const sf::IpAddress SERVER_IP = "127.0.0.1";
int CURRENT_PORT = SERVER_TCP_PORT;

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
		tcp_clients.push_back(std::move(client));
		std::cout << "client " << client.getClientID() << " connected" << std::endl;
		player_number++;
		std::cout << "number of players: " << player_number << std::endl;
		//if (player_number > 4)
		//{
		//	//disconnect(tcp_clients, client, selector);
		//	std::cout << "not enough room, kicking client " << client.getClientID() << std::endl;
		//}
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
				//selector.remove(sender_ref);
				//sender_ref.disconnect();
				//std::cout << "disconnect occurred" << std::endl;
				//std::cout << "Client " << iter->getClientID() << " disconnected" << std::endl;
				//std::cout << "number of clients before: " << tcp_clients.size() << std::endl;
				////tcp_clients.erase(std::remove(tcp_clients.begin(), tcp_clients.end(), iter),iter);
				//tcp_clients.erase(std::remove(tcp_clients.begin(), tcp_clients.end(), *iter), tcp_clients.end());
				////tcp_clients.erase(iter);
				//std::cout << "number of clients after: " << tcp_clients.size() << std::endl;
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
			if (selector.isReady(tcp_listener) && player_number < 2)
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