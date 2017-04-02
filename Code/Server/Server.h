#pragma once
#include <SFML/Network.hpp>
#include <SFML\Graphics.hpp>
#include <memory>
#include <vector>
#include <chrono>
#include "Client.h"

using TcpClient = sf::TcpSocket;
using TcpClientPtr = std::unique_ptr<TcpClient>;
using TcpClients = std::vector<Client>;
using Key = sf::Keyboard::Key;

class Server
{
public:
	Server();
	~Server() = default;
	bool bindServerPort(sf::TcpListener&);
	void connect(sf::TcpListener&, sf::SocketSelector&, TcpClients&);
	void disconnect(TcpClients&, Client&, sf::SocketSelector&);
	void receiveMsg(TcpClients&, sf::SocketSelector&);
	void listen(sf::TcpListener&, sf::SocketSelector&, TcpClients&);
	void runMe();
	void ping(TcpClients&);
	void runGame(TcpClients&);
	void updatePositions(TcpClients&);

private:
	struct ControlKeys
	{
		//Key order: up, left, down, right (WASD)
		std::vector<Key> player1 = { Key::W, Key::A, Key::S, Key::D };
		std::vector<Key> player2 = { Key::Up, Key::Left, Key::Down, Key::Right };
		std::vector<Key> player3 = { Key::Y, Key::G, Key::H, Key::J };
		std::vector<Key> player4 = { Key::P, Key::L, Key::SemiColon , Key::Quote };
	};

	int player_number = 0;
	std::vector<std::vector<Key>> controls_set;
	std::vector<sf::Vector2f> starting_positions = { sf::Vector2f(0.0f, 300.0f), sf::Vector2f(300.0f, 0.0f) };
	std::vector<sf::Vector2f> gridPositions;
	sf::Mutex mutex;
};