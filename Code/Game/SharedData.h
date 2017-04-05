#pragma once
//System libs
#include <memory>
#include <vector>

//SFML
#include <SFML\Network.hpp>
#include <SFML\Graphics.hpp>

using Key = sf::Keyboard::Key;

//Stores width and height of client's window
//Also used by server to create the positions grid
struct WindowSize
{
	//width and height of the client's window
	const static int width = 600;
	const static int height = 600;

	//used by server and client to create (grid_size x grid_size) sized grid
	//Note: grid_size must be even - odd grid_sizes are not accounted for and players can go offscreen
	const static int grid_size = 64;
	const static int grid_points = grid_size * grid_size;
};

enum NetMsg : sf::Uint32
{
	INVALID = 0,
	MOVEDIR = 1,
	GRID_STATE = 2,
	INIT = 3,
	PING = 4,
	PONG = 5,
};

//movement directions for the player
enum PlayerMove : sf::Uint32
{
	UP = 1,
	LEFT = 2,
	DOWN = 3,
	RIGHT = 4,
};

//stores control schemes for the clients
struct Controls
{
	int up = 0;
	int left = 0;
	int down = 0;
	int right = 0;
};

//create struct to store player data in
struct PlayerData
{
	sf::Uint8 client_id;
	sf::Vector2i spawn_pos;
	sf::Vector2i grid_index;
	PlayerMove move_dir;
	PlayerMove start_dir;
	Controls controls;
	bool is_alive = true;
};

struct StartData
{
	//Key order: up, left, down, right (WASD)
	std::vector<Key> up = { Key::W, Key::Up, Key::Y, Key::P };
	std::vector<Key> left = { Key::A, Key::Left, Key::G, Key::L };
	std::vector<Key> down = { Key::S, Key::Down, Key::H, Key::SemiColon };
	std::vector<Key> right = { Key::D, Key::Right, Key::J , Key::Quote };
	std::vector<PlayerMove> start_dir = { RIGHT, LEFT, DOWN, UP };

	//Grid indices for where each player will begin 
	std::vector<sf::Vector2i> starting_positions = { 
		sf::Vector2i(0, WindowSize::grid_size / 2),								//Left-mid
		sf::Vector2i(WindowSize::grid_size - 1, WindowSize::grid_size / 2),		//Right-mid
		sf::Vector2i(WindowSize::grid_size / 2, 0),								//Top-mid
		sf::Vector2i(WindowSize::grid_size / 2, WindowSize::grid_size - 1)		//Bottom-mid
	};
};