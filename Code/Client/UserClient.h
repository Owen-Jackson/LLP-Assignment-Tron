#pragma once
#include <SFML\Network.hpp>
#include <SFML\Graphics.hpp>
#include "Player.h"
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

	Player* getPlayer() { return player.get(); };
	std::vector<Player*> getEnemies() { return enemies; };
	const void addOpponent(Player* opponent);

private:
	std::unique_ptr<Player> player = nullptr;
	std::vector<Player*> enemies;
};