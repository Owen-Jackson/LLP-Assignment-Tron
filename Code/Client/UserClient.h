//System libs
#include <mutex>
#include <vector>

//SFML
#include <SFML\Network.hpp>
#include <SFML\Graphics.hpp>

//Shared from Game
#include <Game\SharedData.h>

using TcpClient = sf::TcpSocket;
using TcpClientPtr = std::unique_ptr<TcpClient>;
using TcpClients = std::vector<TcpClient>;

class UserClient
{
public:
	UserClient();
	~UserClient() = default;

	bool connect(TcpClient&);	//connect to server
	void input(TcpClient&);		//get keyboard input
	void client();				//main method; calls other methods and receives packets from the server

	std::vector<sf::Int32>& getGrid() { return grid; };

	void setControls(Controls&);	//maps keys for the player

private:
	PlayerData my_data;
	std::mutex mutex;
	std::vector<sf::Int32> grid;
};