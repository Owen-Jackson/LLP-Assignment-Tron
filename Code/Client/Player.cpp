#include "Player.h"

Player::Player()
{
	sprite = std::make_unique<sf::RectangleShape>();
	sprite->setSize(sf::Vector2f(50.0f, 50.0f));
	sprite->setFillColor(sf::Color::Cyan);
}

void Player::setData(InitData& new_controls)
{
	player_data.up = new_controls.up;
	player_data.left = new_controls.left;
	player_data.down = new_controls.down;
	player_data.right = new_controls.right;
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
		break;
	default:
		;
	}
	sprite->setPosition(position);
}