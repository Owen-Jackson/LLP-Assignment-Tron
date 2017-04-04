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

sf::TcpSocket& Client::getSocket()
{
	return *socket;
}

void Client::setStartDirection(PlayerMove& spawn_dir)
{
	data.start_dir = spawn_dir;
	data.move_dir = spawn_dir;
}

void Client::setLatency(std::chrono::microseconds delay)
{
	latency = delay;
}

void Client::setSpawn(sf::Vector2i& spawn)
{
	data.spawn_pos = spawn;
	data.grid_index = spawn;
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

void Client::tick()
{
	switch (data.move_dir)
	{
	case PlayerMove::LEFT:
		if (data.grid_index.x % WindowSize::grid_size != 0)
		{
			data.grid_index.x--;
		}
		break;
	case PlayerMove::RIGHT:
		if ((data.grid_index.x + 1) % WindowSize::grid_size != 0)
		{
			data.grid_index.x++;
		}
		break;
	case PlayerMove::UP:
		if (data.grid_index.y - 1 >= 0)
		{
			data.grid_index.y--;
		}
		break;
	case PlayerMove::DOWN:
		if (data.grid_index.y + 1 < WindowSize::grid_size)
		{
			data.grid_index.y++;
		}
		break;
	default:
		;
	}
}

void Client::respawn()
{
	data.grid_index = data.spawn_pos;
	data.move_dir = data.start_dir;
}