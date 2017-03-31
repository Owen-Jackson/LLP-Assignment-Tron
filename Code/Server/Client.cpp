#include "Client.h"
#include <Game/SharedData.h>

Client::Client(sf::TcpSocket* _socket) : socket(_socket)
{
	next_id = 0;
}

Client::Client(Client &&rhs)
{
	this->id = rhs.id;
	this->latency = rhs.latency;
	this->socket = std::move(rhs.socket);
}

Client& Client::operator=(Client&& rhs)
{
	this->id = rhs.id;
	this->latency = rhs.latency;
	this->socket = std::move(rhs.socket);
	return *this;
}

bool Client::operator==(const Client& rhs)
{
	return id == rhs.id;
}

sf::TcpSocket& Client::getSocket()
{
	return *socket;
}

void Client::setLatency(std::chrono::microseconds delay)
{
	latency = delay;
}

void Client::ping()
{
	//send ping packet
	sf::Packet packet;
	packet << NetMsg::PING;
	socket->send(packet);

	//record time sent
	timestamp = std::chrono::steady_clock::now();
}

void Client::pong()
{
	auto end = std::chrono::steady_clock::now();
	latency = std::chrono::duration_cast<std::chrono::microseconds>(end - timestamp);

	latency /= 2;
}