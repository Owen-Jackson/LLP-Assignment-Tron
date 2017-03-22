#pragma once
#include <SFML/Network.hpp>
#include <memory>
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

	void setLatency(std::chrono::microseconds);
	const auto& getPingTime() const { return timestamp; };
	const auto& getLatency() const { return latency; };

	void ping();
	void pong();

private:
	static unsigned int next_id;
	int id = next_id++;

	std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();
	std::chrono::microseconds latency = 100us;
	std::unique_ptr<sf::TcpSocket> socket = nullptr;
};