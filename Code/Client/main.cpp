#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include "UserClient.h"
#include <iostream>
#include <future>
#include <string>
#include <random>

using Key = sf::Keyboard::Key;
sf::Mutex mtx;

void runThread(UserClient* client)
{
	client->client();
}

int main()
{
	srand(unsigned int(time(NULL)));

	sf::RenderWindow window(sf::VideoMode(WindowSize::width, WindowSize::height), "Tron Game");

	//get font for text
	sf::Font font;
	font.loadFromFile("../../Libs/Fonts/arial.ttf");

	std::vector<sf::Color> colours = { sf::Color::Red, sf::Color::Green, sf::Color::Cyan, sf::Color::Yellow };
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

	sf::RectangleShape grid_square;
	grid_square.setSize(sf::Vector2f((float)WindowSize::width / WindowSize::grid_size, (float)WindowSize::height / WindowSize::grid_size));

	//limit framerate
	window.setFramerateLimit(30);

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
		//loop through the grid and draw a rectangle in each place that a player has been in
		mtx.lock();
		for (int i = 0; i < WindowSize::grid_size; i++)
		{
			for (int j = 0; j < WindowSize::grid_size; j++)
			{
				if (user->getGrid()[(i * WindowSize::grid_size) + j] != -1)
				{
					grid_square.setPosition((i * ((float)WindowSize::width/WindowSize::grid_size)), (j *((float)WindowSize::height / WindowSize::grid_size)));
					grid_square.setFillColor(colours[user->getGrid()[(WindowSize::grid_size * i) + j] % 4]);
					window.draw(grid_square);
				}
			}
		}
		mtx.unlock();
		//grid_square.setFillColor(sf::Color::Blue);
		//window.draw(grid_square);


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