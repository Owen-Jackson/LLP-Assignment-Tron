#pragma once
#include <SFML\Network.hpp>
#include <SFML\Graphics.hpp>
#include <Game\SharedData.h>
#include <memory>
#include <vector>

using TcpClient = sf::TcpSocket;
using TcpClientPtr = std::unique_ptr<TcpClient>;
using TcpClients = std::vector<TcpClient>;

class UserClient
{
public:
	UserClient();
	~UserClient() = default;

	bool connect(TcpClient&);
	void input(TcpClient&);
	void client();
	void addOpponent(sf::Packet& packet);


	void createPlayers();
	void setControls(InitData&);
	std::vector<std::unique_ptr<sf::RectangleShape>>& getPlayerSprites() { return player_sprites; };

private:
	InitData controls;
	PlayerData my_data;
	StartData start_data;
	std::vector<std::unique_ptr<sf::RectangleShape>> player_sprites;
	sf::Mutex mutex;
};