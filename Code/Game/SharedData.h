#pragma once
#include <SFML/Network.hpp>

enum NetMsg : sf::Int32
{
	INVALID = 0,
	CHAT = 1,
	PLAYER = 2,
	PING = 3,
	PONG = 4,
};