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
	const static int grid_size = 32;	//used by server to create (grid_size x grid_size) sized grid and client when moving
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
	NEWPLAYER = 7,
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
	sf::Uint32 client_id;
	float xPos;
	float yPos;
	PlayerMove move_dir;
};

struct StartData
{
	//Key order: up, left, down, right (WASD)
	std::vector<Key> up = { Key::W, Key::Up, Key::Y, Key::P };
	std::vector<Key> left = { Key::A, Key::Left, Key::G, Key::L };
	std::vector<Key> down = { Key::S, Key::Down, Key::H, Key::SemiColon };
	std::vector<Key> right = { Key::D, Key::Right, Key::J , Key::Quote };
	std::vector<sf::Color> colours = { sf::Color::Red, sf::Color::Green, sf::Color::Cyan, sf::Color::Yellow };
	std::vector<sf::Vector2f> starting_positions = { sf::Vector2f(0.0f, float(WindowSize::height / 2)),
													 sf::Vector2f(float(WindowSize::width - (WindowSize::width / WindowSize::grid_size)), float(WindowSize::height / 2)),
													 sf::Vector2f(float(WindowSize::width / 2), 0),
													 sf::Vector2f(float(WindowSize::width / 2), float(WindowSize::height - (WindowSize::height / WindowSize::grid_size))) };
};