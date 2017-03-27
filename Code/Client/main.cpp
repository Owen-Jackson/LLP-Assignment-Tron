#include <SFML/Graphics.hpp>
#include "UserClient.h"
#include "iostream"
#include <future>

int main()
{
	sf::RenderWindow window(sf::VideoMode(1024, 720), "SFML works!");
	sf::CircleShape shape(100.f);
	shape.setFillColor(sf::Color::Green);
	UserClient user;
	window.setFramerateLimit(60);

	//Connect the client to the server before opening the window so that it doesn't get stuck
	auto handle = std::async(std::launch::async, [&]
	{
		user.client();
	});

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

			else if (event.type == sf::Event::KeyPressed)
			{
				
					shape.move(5.0f, 0.0f);
				
			}
		}

		window.clear();
		window.draw(shape);
		window.display();
	}

	return 0;
}