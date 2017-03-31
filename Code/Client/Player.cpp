#include "Player.h"

Player::Player()
{
	sprite = std::make_unique<sf::RectangleShape>();
	sprite->setSize(sf::Vector2f(100.0f, 50.0f));
	sprite->setFillColor(sf::Color::Cyan);
}

void Player::Tick()
{
	switch (move_dir)
	{
	case PlayerMove::LEFT:
		position.x -= move_speed;
		break;
	case PlayerMove::RIGHT:
		position.x += move_speed;
		break;
	case PlayerMove::UP:
		position.y -= move_speed;
		break;
	case PlayerMove::DOWN:
		position.y += move_speed;
		sprite->setFillColor(sf::Color::Green);
		break;
	default:
		;
	}
	sprite->setPosition(position);
}