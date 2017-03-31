#include <SFML\Graphics.hpp>
#include <Game\SharedData.h>
#include <memory>


class Player
{
public:
	Player();
	~Player() = default;

	void setMoveDir(PlayerMove new_dir) { move_dir = new_dir; };
	void setPosition(sf::Vector2f new_pos) { position = new_pos; };

	PlayerMove getMoveDir() { return move_dir; };
	sf::Vector2f getPosition() { return position; };
	sf::RectangleShape* getSprite() { return sprite.get(); };
	void Tick();

private:
	PlayerMove move_dir = PlayerMove::NONE;
	sf::Vector2f position;
	float move_speed = 2.0f;
	std::unique_ptr<sf::RectangleShape> sprite;
};