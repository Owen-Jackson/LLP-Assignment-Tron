//System libs
#include <memory>
#include <utility>
#include <vector>
#include <chrono>

//SFML
#include <SFML\Network.hpp>

//Shared from Game
#include <Game\SharedData.h>

using namespace std::chrono_literals;

class Client
{
public:
	Client() = default;
	Client(sf::TcpSocket*, sf::Uint8 id);
	Client(Client&&);
	~Client() = default;

	Client& operator=(Client&&);
	bool operator == (const Client& rhs);

	//Setters
	void setStartDirection(PlayerMove&);
	void setDirection(PlayerMove& new_dir) { data.move_dir = new_dir; };
	void setLatency(std::chrono::microseconds);
	void setSpawn(sf::Vector2i& spawn);
	void killMe() { data.is_alive = false; };

	//Getters
	sf::TcpSocket& getSocket();
	sf::Vector2i& getSpawnPoint() { return data.spawn_pos; };
	sf::Vector2i& getIndexPosition() { return data.grid_index; };
	const auto& getPingTime() const { return timestamp; };
	const auto& getLatency() const { return latency; };
	sf::Uint8 getClientID() const { return id; };
	bool isAlive() const { return data.is_alive; };

	//Ping/Pong for messaging latency to server
	void ping();
	void pong();

	//Game loop updates
	void tick();
	void respawn();

private:
	sf::Uint8 id = 0;

	//latency attributes
	std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();
	std::chrono::microseconds latency = 100us;

	std::unique_ptr<sf::TcpSocket> socket = nullptr;
	PlayerData data;	//stores all of the needed client data
};