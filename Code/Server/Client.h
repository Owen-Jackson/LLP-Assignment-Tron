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

	void setDirection(PlayerMove& new_dir) { data.move_dir = new_dir; };
	void setLatency(std::chrono::microseconds);
	void setPosition(float x, float y);
	void setSpawn(float x, float y);

	sf::Vector2f& getPosition();
	PlayerMove& getMoveDirection() { return data.move_dir; };
	const auto& getPingTime() const { return timestamp; };
	const auto& getLatency() const { return latency; };
	int getClientID() const { return id; };

	void ping();
	void pong();

	//Game loop updates
	void Tick();
	void checkCollisions();

private:
	static unsigned int next_id;
	int id = next_id++;

	std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();
	std::chrono::microseconds latency = 100us;
	std::unique_ptr<sf::TcpSocket> socket = nullptr;
	float move_speed = (float)WindowSize::width / (float)WindowSize::grid_size;
	float sprite_bound_max = float(WindowSize::width / WindowSize::grid_size);
	PlayerData data;
};