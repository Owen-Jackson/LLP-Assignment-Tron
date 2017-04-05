//System libs
#include <memory>
#include <vector>
#include <chrono>
#include <queue>
#include <functional>

//SFML
#include <SFML\Network.hpp>
#include <SFML\Graphics.hpp>

//User defined
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
	//Server logic
	bool bindServerPort(sf::TcpListener&);
	void connect(sf::TcpListener&, sf::SocketSelector&, TcpClients&);		//connects new clients
	void disconnect(TcpClients&, Client&, sf::SocketSelector&);				//disconnects current clients
	void receiveMsg(TcpClients&, sf::SocketSelector&);						//receieve and send packets
	void listen(sf::TcpListener&, sf::SocketSelector&, TcpClients&);		//listen for packets and new client connections
	void runMe();															//setup threads and run server and game loops
	void ping(TcpClients&);													//ping all clients to get their latency

	//Game Logic
	void runGame(TcpClients&);			//main game loop
	void sendPositions(TcpClients&);	//send the current grid state to all clients
	void updateGrid(TcpClients&);		//update the grid based on cliend id and index
	void refreshGrid(TcpClients&);		//reset players and grid positions
	void killPlayer(Client&);			//remove player (until end of round)

private:
	sf::Uint8 player_number = 0;	//total number of players in the game
	sf::Uint8 next_id = 0;			//the id to be given to the next client to connect
	std::priority_queue<sf::Uint8, std::vector<sf::Uint8>, std::greater<sf::Uint8>> disconnected_player_ids = {};	//stores the ids of clients who left mid-game
	StartData start_data;
	std::vector<sf::Int32> grid;	//stores ints based on client ids
	sf::Mutex mutex;
};