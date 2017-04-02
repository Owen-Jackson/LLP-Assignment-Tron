#pragma once
#include <SFML\Network.hpp>
#include <SFML\Graphics.hpp>
#include <memory>

//Stores width and height of client's window
//Also used by server to create the positions grid
struct WindowSize
{
	const static int width = 600;
	const static int height = 600;
	const static int grid_size = 16;	//used by server to create (grid_size x grid_size) sized grid and client when moving
};

enum NetMsg : sf::Uint32
{
	INVALID = 0,
	CHAT = 1,
	MOVEDIR = 2,
	POSITIONS = 3,
	INIT = 4,
	PING = 5,
	PONG = 6,
};

enum PlayerMove : sf::Uint32
{
	NONE = 0,
	LEFT = 1,
	RIGHT = 2,
	UP = 3,
	DOWN = 4,
};

struct InitData
{
	//Control scheme for the client
	int up;
	int left;
	int down;
	int right;
};

//create struct to store player data in
struct PlayerData
{
	std::unique_ptr<sf::RectangleShape> sprite;
	float xPos;
	float yPos;
	PlayerMove move_dir;
};