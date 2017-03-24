#include "Server.h"
#include <Game/SharedData.h>
#include <iostream>
#include <string>

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
	if (tcp_listener.accept(client_ref) == sf::Socket::Done)
	{
		selector.add(client_ref);

		auto client = Client(client_ptr);
		tcp_clients.push_back(std::move(client));
		std::cout << "client connected" << std::endl;
	}
}

void Server::receiveMsg(TcpClients& tcp_clients, sf::SocketSelector& selector)
{
	//loop through all tcp clients to find the sender
	for (auto& iter : tcp_clients)
	{
		auto& sender_ref = iter.getSocket();
		if (selector.isReady(sender_ref))
		{
			sf::Packet packet;
			iter.getSocket().receive(packet);
			int header = 0;
			packet >> header;

			NetMsg msg = static_cast<NetMsg>(header);

			if (msg == NetMsg::CHAT)
			{
				std::string string;
				packet >> string;

				//print out the received string
				std::cout << "Net msg: " << string << std::endl;
				std::cout << "Latency: " << iter.getLatency().count()
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
				iter.pong();
				std::cout << "Client latency: " << iter.getLatency().count() << "us" << std::endl;
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
		//client.setLatency(std::chrono::duration_cast<std::chrono::microseconds>(delta));
		if (delta > timeout)
		{
			client.ping();
		}
	}
}