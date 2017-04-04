#include <SFML\Network.hpp>
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
	void sendPositions(TcpClients&);
	void refreshGrid(const int& client_id);
	void updateGrid(TcpClients&);

private:
	sf::Uint8 player_number = 0;
	StartData start_data;
	std::vector<sf::Int32> grid;
	sf::Mutex mutex;
};