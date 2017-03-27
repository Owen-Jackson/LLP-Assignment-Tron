#include <SFML\Graphics.hpp>

class Player
{
public:
	Player() = default;
	~Player() = default;

	//void Tick();
private:
	float move_speed = 2.0f;
	sf::RectangleShape sprite;
};