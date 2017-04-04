#pragma once
#include <SFML\Network.hpp>
#include <SFML\Graphics.hpp>
#include <memory>

using Key = sf::Keyboard::Key;

//Stores width and height of client's window
//Also used by server to create the positions grid
struct WindowSize
{
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
	CHAT = 1,
	MOVEDIR = 2,
	GRID_STATE = 3,
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

struct Controls
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
	sf::Uint8 client_id;
	sf::Vector2i spawn_pos;
	sf::Vector2i grid_index;
	PlayerMove move_dir;
	Controls controls;
};

struct StartData
{
	//Key order: up, left, down, right (WASD)
	std::vector<Key> up = { Key::W, Key::Up, Key::Y, Key::P };
	std::vector<Key> left = { Key::A, Key::Left, Key::G, Key::L };
	std::vector<Key> down = { Key::S, Key::Down, Key::H, Key::SemiColon };
	std::vector<Key> right = { Key::D, Key::Right, Key::J , Key::Quote };
	std::vector<sf::Color> colours = { sf::Color::Red, sf::Color::Green, sf::Color::Cyan, sf::Color::Yellow };

	//Grid indices for where each player will begin 
	std::vector<sf::Vector2i> starting_positions = { 
		sf::Vector2i(0, WindowSize::grid_size / 2), 
		sf::Vector2i(WindowSize::grid_size - 1, WindowSize::grid_size / 2),
		sf::Vector2i(WindowSize::grid_size / 2, 0), 
		sf::Vector2i(WindowSize::grid_size / 2, WindowSize::grid_size - 1) 
	};
};