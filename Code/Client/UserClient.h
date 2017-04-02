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

	void setControls(InitData&);
	std::vector<PlayerData>& getPlayers() { return players; };
	PlayerData& getPlayer() { return my_player; };
	std::unique_ptr<sf::RectangleShape>& getPlayerSprite() { return my_player.sprite; };

private:
	sf::Uint8 move_speed = 2.0f;
	PlayerData my_player;
	InitData controls;
	std::vector<PlayerData> players;
	sf::Mutex mutex;
};