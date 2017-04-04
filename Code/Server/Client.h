#include <SFML\Network.hpp>
#include <Game\SharedData.h>
#include <memory>
#include <utility>
#include <vector>
#include <chrono>

using namespace std::chrono_literals;

class Client
{
public:
	Client() = default;
	Client(sf::TcpSocket*);
	Client(Client&&);
	~Client() = default;

	Client& operator=(Client&&);
	bool operator == (const Client& rhs);
	sf::TcpSocket& getSocket();

	void setStartDirection(PlayerMove&);
	void setDirection(PlayerMove& new_dir) { data.move_dir = new_dir; };
	void setLatency(std::chrono::microseconds);
	void setSpawn(sf::Vector2i& spawn);

	sf::Vector2i& getSpawnPoint() { return data.spawn_pos; };
	sf::Vector2i& getIndexPosition() { return data.grid_index; };
	PlayerMove& getMoveDirection() { return data.move_dir; };
	const auto& getPingTime() const { return timestamp; };
	const auto& getLatency() const { return latency; };
	int getClientID() const { return id; };

	void ping();
	void pong();

	//Game loop updates
	void tick();
	void respawn();
private:
	static unsigned int next_id;
	int id = next_id++;

	std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();
	std::chrono::microseconds latency = 100us;
	std::unique_ptr<sf::TcpSocket> socket = nullptr;
	PlayerData data;
};