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

bool Client::Tick(std::vector<sf::Int32>& grid)
{
	switch (data.move_dir)
	{
	case PlayerMove::LEFT:
		if (data.grid_index.x % WindowSize::grid_size == 0 || grid[(data.grid_index.y * WindowSize::grid_size) + data.grid_index.x] != -1)
		{
			return false;
		}
		data.grid_index.x--;
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
	return true;
	//return checkCollisions(grid);
}

bool Client::checkCollisions(std::vector<sf::Int32>& grid)
{
	if (grid[(data.grid_index.y * WindowSize::grid_size) + data.grid_index.x] != -1)
	{
		data.grid_index = data.spawn_pos;
		return false;
	}
	return true;
}