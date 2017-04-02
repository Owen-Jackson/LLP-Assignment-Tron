#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include "UserClient.h"
#include <iostream>
#include <future>
#include <string>

using Key = sf::Keyboard::Key;

void runThread(UserClient* client)
{
	client->client();
}

int main()
{
	sf::RenderWindow window(sf::VideoMode(WindowSize::width, WindowSize::height), "Tron Game");

	//get font for text
	sf::Font font;
	font.loadFromFile("../../Libs/Fonts/arial.ttf");

	//create text
	//*sf::Text text;
	//std::string str;
	//text.setFont(font);
	//text.setPosition(300.0f, 300.0f);*/
	//text.setCharacterSize(30);
	//text.setFillColor(sf::Color::White);
	//text.setStyle(sf::Text::Regular);

	//create client to connect to server
	UserClient* user = new UserClient();

	//limit framerate
	window.setFramerateLimit(60);

	//Connect the client to the server in a separate thread
	std::thread thread(&runThread, user);
	thread.detach();

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				window.close();
			}

			//Text entered event for chat (stretch goal)
			/*if (event.type == sf::Event::TextEntered)
			{
				if (event.text.unicode < 128)
				{
					str += static_cast<char>(event.text.unicode);
					text.setString(str);
				}
			}*/
		}

		window.clear();
		for (auto& player : user->getPlayers())
		{
			window.draw(*(player.sprite.get()));
		}
		//window.draw(*(user->getPlayerSprite()));
		//window.draw(shape);
		//window.draw(text);
		//window.draw(*(user->getPlayer()->getSprite()));
		//window.draw(*(user->getOpponent()->getSprite()));
		/*for (auto& enemy : user->getEnemies())
		{
			window.draw(enemy->getSprite());
		}*/
		window.display();
	}

	delete user;
	return 0;
}