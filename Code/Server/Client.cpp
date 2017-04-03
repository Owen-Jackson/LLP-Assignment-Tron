#include "Client.h"
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
	this->data = rhs.data;
}

Client& Client::operator=(Client&& rhs)
{
	this->id = rhs.id;
	this->latency = rhs.latency;
	this->socket = std::move(rhs.socket);
	this->data = rhs.data;
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

void Client::setSpawn(float x, float y)
{
	data.spawn_pos.x = x;
	data.spawn_pos.y = y;
}

sf::TcpSocket& Client::getSocket()
{
	return *socket;
}

sf::Vector2f& Client::getPosition()
{
	sf::Vector2f pos = sf::Vector2f(data.xPos, data.yPos);
	return pos;
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
	checkCollisions();
}

void Client::checkCollisions()
{
	if (data.xPos + sprite_bound_max > WindowSize::width || data.xPos < 0
		|| data.yPos + sprite_bound_max > WindowSize::height || data.yPos < 0)
	{
		data.xPos = data.spawn_pos.x;
		data.yPos = data.spawn_pos.y;
	}
}