#include "Client.h"
#include <Game/SharedData.h>
#include <iostream>

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

void Client::setPosition(float x, float y)
{
	data.xPos = x;
	data.yPos = y;
}

sf::TcpSocket& Client::getSocket()
{
	return *socket;
}

std::pair<float, float>& Client::getPosition()
{
	std::pair<float, float> coords;
	coords.first = data.xPos;
	coords.second = data.yPos;
	return coords;
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

void Client::Tick()
{
	switch (data.move_dir)
	{
	case PlayerMove::LEFT:
		data.xPos -= move_speed;
		break;
	case PlayerMove::RIGHT:
		data.xPos += move_speed;
		break;
	case PlayerMove::UP:
		data.yPos -= move_speed;
		break;
	case PlayerMove::DOWN:
		data.yPos += move_speed;
		break;
	default:
		;
	}
}

void Client::checkCollisions()
{
	/*if (data.xPos + data.sprite->getSize().x)
	{
		;
	}*/
}