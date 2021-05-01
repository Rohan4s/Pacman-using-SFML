#include<SFML/Graphics.hpp>
#include<SFML/Window.hpp>
#include<iostream>
#include<fstream>
#include<cctype>
#include<vector>
#include<string>



#define cellSize 15   // each cell is (15*15) pixels
#define mapSize 40+1  // a total of 41 rows and columns


bool isReachable(bool grid[][41], char direction, int currentPositionX, int currentPositionY ) {  // Returns true if the 4 adjacent cells dont have walls
	
	int x, y;
	x = currentPositionX / cellSize;
	y = currentPositionY / cellSize;

	if (direction == 'W')
		return grid[x][y - 1];
	else if (direction == 'S')
		return grid[x][y + 1];
	else if (direction == 'A')
		return grid[x - 1][y];
	else if (direction == 'D')
		return grid[x + 1][y];
	else
		return 0;
};

//std::pair<int, int> pairCoordinate(sf::Vector2f a) {
//	int x, y;
//	x = (int)a.x;
//	y = (int)a.y;
//	return std::make_pair(x, y);
//}

int main() {

	sf::RenderWindow window(sf::VideoMode(cellSize*mapSize, cellSize*mapSize), "Pacman");
	window.setFramerateLimit(60);

	int inputDelay = 20, autoMovement=10;
	char movementDir='0';  // initializing player as stationary
	float movementSpeed = cellSize; // Moves a cell each time the player moves


	//food

	sf::Sprite food;
	sf::Texture foodText;
	if (!foodText.loadFromFile("Resources/Texture/food.png"))
		std::cout << "failed to load food.png";
	food.setTexture(foodText);
	food.setScale(cellSize / food.getGlobalBounds().width , cellSize / food.getGlobalBounds().height);
	std::vector < sf::Sprite > foods;

	
	//Map
	//const int mapSize = 40 + 1;
	
	bool notWall[mapSize][mapSize]; 

	for (int i = 0; i < mapSize; i++)
		for (int j = 0; j < mapSize; j++)
			notWall[i][j] = 1; // Initializing the array as 1 meaning there is no wall
	
	std::vector<std::vector<sf::RectangleShape>> map;


	map.resize(mapSize, std::vector<sf::RectangleShape>());
	for (int i = 0; i < mapSize; i++)
	{
		map[i].resize(mapSize, sf::RectangleShape());
		for (int j = 0; j < mapSize; j++)
		{
			map[i][j].setSize(sf::Vector2f(cellSize,cellSize));
			map[i][j].setFillColor(sf::Color::White);
			/*map[i][j].setOutlineThickness(1.f);
			map[i][j].setOutlineColor(sf::Color::Black);*/
			map[i][j].setPosition(i * cellSize, j * cellSize);
		}
	}
	std::ifstream mapInfo("Resources/Map/map.txt");
	if (!mapInfo.is_open()) 
		std::cout << "Error loading pacmanMap.txt" << '\n';
	sf::Vector2f pacmanSpawn; // Stores the coordinate where pacman spawns at the start of the round
	if(mapInfo.is_open())
	{
		std::string row;
		int pos = 0;
		while (std::getline(mapInfo, row))
		{
			for (int i = 0; i < 40; i++)
			{
				if (row[i] == 'B') // if there is a wall
				{
					notWall[pos][i] = 0;
					map[pos][i].setFillColor(sf::Color::Black);
				}
					
				else if (row[i] == 'P')
				{
					pacmanSpawn.x = pos * cellSize; // change needed
					pacmanSpawn.y = i * cellSize;
				}
				else if(rand()%6==1) // randomly spawns food
				{
					foods.push_back(food);
					foods[foods.size()-1].setPosition(sf::Vector2f(pos*cellSize,i*cellSize));
				}
			}
			pos++;
			row.clear();
		}
		std::cout << "grid " << notWall[34][37]<<'\n';

		//while (!mapInfo.eof())
		//{
		//	mapInfo >> row;
		//	std::cout<<"pos = "<<pos<<"=> " << row;
		//	for (int i = 0; i < row.size(); i++)
		//	{
		//		if (row[i] == 'B')
		//		{
		//			std::cout << "i,j = " << pos << ", " << i << '\n';
		//			map[pos][i].setFillColor(sf::Color::Red);
		//		}
		//	}
		//	pos++;
		//}
	}


	

	//player

	sf::Sprite player;
	sf::Texture playerTextUp, playerTextDown, playerTextLeft, playerTextRight;
	if (!playerTextUp.loadFromFile("Resources/Texture/playerUp.png"))
		std::cout << "Failed to load playerUp.png";
	if (!playerTextDown.loadFromFile("Resources/Texture/playerDown.png"))
		std::cout << "Failed to load playerDown.png";
	if (!playerTextLeft.loadFromFile("Resources/Texture/playerRight.png"))
		std::cout << "Failed to load playerLeft.png";
	if (!playerTextRight.loadFromFile("Resources/Texture/playerRight.png"))
		std::cout << "Failed to load playerRight.png";

	player.setTexture(playerTextRight);
	float playerSize = cellSize;
	player.setScale(playerSize/1200,playerSize/1403 );
	player.setPosition(pacmanSpawn);

	std::cout << pacmanSpawn.x << pacmanSpawn.y << '\n';

	std::cout << "Works! " << player.getGlobalBounds().height << " " << player.getGlobalBounds().width;

	// ghost
	sf::Sprite Blinky, Pinky, Inky, Clyde;
	sf::Texture BlinkyText, PinkyText, InkyText, ClydeText;
	if (!BlinkyText.loadFromFile("Resources/Texture/blinky.png"))
		std::cout << "failed to load blinky.png";
	Blinky.setTexture(BlinkyText);
	Blinky.setScale(cellSize/202.0,cellSize/201.0 );
	std::cout << "blinky= " << Blinky.getGlobalBounds().height << " " << Blinky.getGlobalBounds().width << '\n';

	if (!ClydeText.loadFromFile("Resources/Texture/clyde.png"));
		std::cout << "failed to load clyde.png";
	Clyde.setTexture(ClydeText);
	Clyde.setScale(cellSize/Clyde.getGlobalBounds().width,cellSize/Clyde.getGlobalBounds().height );
	std::cout << "clyde = " << Clyde.getGlobalBounds().height << " " << Clyde.getGlobalBounds().width << '\n';
	Clyde.setPosition(sf::Vector2f(window.getSize().x - Clyde.getGlobalBounds().width, 0.f));

	if (!InkyText.loadFromFile("Resources/Texture/inky.png"));
		std::cout << "failed to load inky.png";
	Inky.setTexture(InkyText);
	Inky.setScale(cellSize/Inky.getGlobalBounds().width ,cellSize/Inky.getGlobalBounds().height );
	std::cout << "inky= " << Inky.getGlobalBounds().height << " " << Inky.getGlobalBounds().width << '\n';
	Inky.setPosition(sf::Vector2f(0.f, window.getSize().y));
	



	// loop

	while (window.isOpen() ) {
		
		// handle events
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed() || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
				window.close();
			}


		//Player update

		inputDelay--;
		autoMovement--;
		int posX = player.getPosition().x;
		int posY = player.getPosition().y;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) && inputDelay <= 0 && posX > 0 && isReachable(notWall, 'A', posX, posY ))
		{	
			player.move(-1 * movementSpeed, 0.f);
			movementDir = 'A';
			player.setTexture(playerTextLeft); // issue
			inputDelay = autoMovement = 10;
			std::cout << "x = " << player.getPosition().x << " y = " << player.getPosition().y << '\n';
		}

		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) && inputDelay <= 0 && posX < cellSize*mapSize - player.getGlobalBounds().width && isReachable(notWall, 'D',posX,posY ))
		{
			player.move(movementSpeed, 0.f);
			movementDir = 'D';
			player.setTexture(playerTextRight);
			inputDelay = autoMovement = 10;
		}

		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) && inputDelay <= 0 && posY >0 && isReachable(notWall, 'W', posX,posY))
		{
			player.move(0.f, -1 * movementSpeed);
			movementDir = 'W';
			player.setTexture(playerTextUp);
			inputDelay = autoMovement = 10;
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) && inputDelay<=0 && posY + player.getGlobalBounds().height  < cellSize * mapSize && isReachable(notWall, 'S', posX,posY)) {
			player.move(0.f, movementSpeed);
			movementDir = 'S';
			player.setTexture(playerTextDown);
			inputDelay = autoMovement = 10;
			std::cout << "x = " << player.getPosition().x << " y = " << player.getPosition().y<<'\n' ;
		}
		else if(autoMovement<=0)
		{
			autoMovement = 10;
			if (movementDir == 'W' &&  posY > 0 && isReachable(notWall, 'W', posX,posY))
				player.move(0.f, -1 * movementSpeed);

			else if (movementDir == 'S' && posY + player.getGlobalBounds().height + 1 < cellSize*mapSize && isReachable(notWall, 'S', posX,posY))
				player.move(0.f, movementSpeed);

			else if (movementDir == 'A' && posX > 0 && isReachable(notWall, 'A', posX,posY))
				player.move(-1 * movementSpeed, 0.f);

			else if (movementDir == 'D' && posX < cellSize * mapSize - player.getGlobalBounds().width && isReachable(notWall, 'D', posX,posY))
				player.move(movementSpeed, 0.f);
				
		}

		// Food update
		for (int i = 0; i < foods.size(); i++)
			if (foods[i].getGlobalBounds().intersects(player.getGlobalBounds()))
				foods.erase(foods.begin() + i);

		window.clear(sf::Color::Transparent);

		// 

		//draw
		for (int i = 0; i < mapSize; i++)
		{
			for (int j = 0; j < mapSize-2; j++)
			{
				window.draw(map[i][j]);
			}
		}
		window.draw(player);
		window.draw(Blinky);
		window.draw(Inky);
		window.draw(Clyde);

		for (int i = 0; i < foods.size(); i++)
			window.draw(foods[i]);
		window.display();
	}
	return 0;
}

