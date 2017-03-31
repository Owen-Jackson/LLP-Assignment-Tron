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

enum PlayerMove : sf::Int32 
{
	NONE = 0,
	LEFT = 1,
	RIGHT = 2,
	UP = 3,
	DOWN = 4,
};