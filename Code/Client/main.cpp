//System libs
#include <iostream>
#include <thread>
#include <mutex>

//Sfml
#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>

//User defined
#include "UserClient.h"

using Key = sf::Keyboard::Key;
std::mutex mtx;

void runThread(UserClient* client)
{
	client->client();
}

int main()
{
	//create game window
	sf::RenderWindow window(sf::VideoMode(WindowSize::width, WindowSize::height), "Tron Game");

	//stores colours to represent each player with
	std::vector<sf::Color> colours = { sf::Color::Red, sf::Color::Green, sf::Color::Cyan, sf::Color::Yellow };

	//create client to connect to server
	UserClient* user = new UserClient();

	//create base shape to draw in the grid
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
					grid_square.setFillColor(colours[user->getGrid()[(WindowSize::grid_size * i) + j] % 4]);	//errors sometimes encountered here (mutex scope?)
					window.draw(grid_square);
				}
			}
		}
		mtx.unlock();
		window.display();
	}

	delete user;
	return 0;
}