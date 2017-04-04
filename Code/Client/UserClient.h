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

	std::vector<sf::Int32>& getGrid() { return grid; };

	void setControls(Controls&);

private:
	PlayerData my_data;
	StartData start_data;
	sf::Mutex mutex;
	std::vector<sf::Int32> grid;
};