#include<SFML/Graphics.hpp>
#include<SFML/Window.hpp>
#include<iostream>
#include<fstream>
#include<cctype>
#include<vector>
#include<string>
#include<set>
#include<stack>
#include<time.h>
#include<stdlib.h>
#include<cstdio>
#include<random>
#define cellSize 15   // each cell is (15*15) pixels
#define mapSize 40+1  // a total of 41 rows and columns

class node {
	public:
		std::pair<int,int> parent = std::make_pair(-1,-1);
		int f=INT_MAX, g = INT_MAX, h = INT_MAX;// f = estimated length of source to destination on a path = g+h . g = source to current node. h = estimated distance between current node and dest
	//bool operator < (const node& rhs) const { return f < rhs.f; }
};

//Some global variables frequently used by functions
int newRow, newCol;
char movementDir = '0';  // initializing player as stationary
bool foundDest = false;
bool notWall[mapSize+2][mapSize+2];
node cellDetails[mapSize + 2][mapSize + 2];
bool closedList[mapSize + 2][mapSize + 2]; // contains nodes that have been fully explored
std::set <std::pair<int, std::pair<int, int> > > openList; // Contains f and the coordinate of a cell
std::stack< std::pair<int, int> > path;
std::pair<int, std::pair<int, int> > parentCell;
sf::Texture playerTextIdle,playerTextUp, playerTextDown, playerTextLeft, playerTextRight;
sf::Texture BlinkyText, PinkyText, InkyText, ClydeText;

//functions

bool isReachable(char direction, int currentPositionX, int currentPositionY);   // Returns true if the specified adjacent cells dont have walls
void tracePath(std::pair<int, int> dest); // puts the shortest path in a stack named Path
void aStar(std::pair<int, int> source, std::pair<int, int> dest); // finds the shortest path from source to dest
void updateBlinky(sf::Sprite& Blinky,sf::Sprite& player);
void updatePinky(sf::Sprite& Pinky, sf::Sprite& player, const char& direction); //direction is the way player is moving
void updateClyde(sf::Sprite& Clyde, sf::Sprite& player);
void moveTowards(sf::Sprite& Entity, char dir); // player/ghost moves toward said direction
bool moveByInput(sf::Sprite& player);
void frightenedMove(sf::Sprite& Entity);

int main() {

	sf::RenderWindow window(sf::VideoMode(cellSize*mapSize, cellSize*mapSize), "Pacman");
	window.setFramerateLimit(60);

	int inputDelay = 20, autoMovement=10, ghostUpdateRate = 20;
	float movementSpeed = cellSize; // Moves a cell each time the player moves
	srand(time(0));

	//food

	sf::Sprite food;
	sf::Texture foodText;
	if (!foodText.loadFromFile("Resources/Texture/food.png"))
		std::cout << "failed to load food.png";
	food.setTexture(foodText);
	food.setScale(cellSize / food.getGlobalBounds().width , cellSize / food.getGlobalBounds().height);
	std::vector < sf::Sprite > foods;

	
	//Map
	 

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
	std::pair<int, int> pacmanSpawn; // Stores the coordinate where pacman spawns at the start of the round
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
					pacmanSpawn.first = pos * cellSize; // change needed
					pacmanSpawn.second = i * cellSize;
				}
				else if(rand()%6==1) // randomly spawns food
				{
					foods.push_back(food);
					foods[foods.size()-1].setPosition((pos*cellSize),(i*cellSize));
				}
			}
			pos++;
			row.clear();
		}

	}



	//player

	sf::Sprite player;

	if (!playerTextIdle.loadFromFile("Resources/Texture/Player.png"))
		std::cout << "Failed to load player.png";
	if (!playerTextUp.loadFromFile("Resources/Texture/playerUp.png"))
		std::cout << "Failed to load playerUp.png";
	if (!playerTextDown.loadFromFile("Resources/Texture/playerDown.png"))
		std::cout << "Failed to load playerDown.png";
	if (!playerTextLeft.loadFromFile("Resources/Texture/playerLeft.png"))
		std::cout << "Failed to load playerLeft.png";
	if (!playerTextRight.loadFromFile("Resources/Texture/playerRight.png"))
		std::cout << "Failed to load playerRight.png";

	player.setTexture(playerTextIdle);
	player.setScale(cellSize / player.getGlobalBounds().width, cellSize / player.getGlobalBounds().height);
	player.setPosition(pacmanSpawn.first,pacmanSpawn.second);

	std::cout << pacmanSpawn.first << pacmanSpawn.second << '\n';

	std::cout << "Works! " << player.getGlobalBounds().height << " " << player.getGlobalBounds().width;

	// ghost
	sf::Sprite Blinky, Pinky, Inky, Clyde;
	if (!BlinkyText.loadFromFile("Resources/Texture/blinkyDown.png"))
		std::cout << "failed to load blinky.png";
	Blinky.setTexture(BlinkyText);
	Blinky.setScale(cellSize/Blinky.getGlobalBounds().width,cellSize/Blinky.getGlobalBounds().height);
	Blinky.setPosition(pacmanSpawn.first,pacmanSpawn.second);
	Blinky.setPosition(12*15,34*15);

	std::cout << "blinky= " << Blinky.getGlobalBounds().height << " " << Blinky.getGlobalBounds().width << '\n';

	if (!ClydeText.loadFromFile("Resources/Texture/clydeUp.png"))
		std::cout << "failed to load clyde.png";
	Clyde.setTexture(ClydeText);
	Clyde.setScale(cellSize/Clyde.getGlobalBounds().width,cellSize/Clyde.getGlobalBounds().height );
	std::cout << "clyde = " << Clyde.getGlobalBounds().height << " " << Clyde.getGlobalBounds().width << '\n';
	Clyde.setPosition(35*cellSize,25*cellSize);

	if (!InkyText.loadFromFile("Resources/Texture/inky.png"))
		std::cout << "failed to load inky.png";
	Inky.setTexture(InkyText);
	Inky.setScale(cellSize/Inky.getGlobalBounds().width ,cellSize/Inky.getGlobalBounds().height );
	std::cout << "inky= " << Inky.getGlobalBounds().height << " " << Inky.getGlobalBounds().width << '\n';
	Inky.setPosition(35*cellSize,25*cellSize);
	
	if (!PinkyText.loadFromFile("Resources/Texture/pinkyDown.png"))
		std::cout << "failed to laod pinky.png";
	Pinky.setTexture(PinkyText);
	Pinky.setScale(cellSize / Pinky.getGlobalBounds().width, cellSize / Pinky.getGlobalBounds().height);
	Pinky.setPosition(Inky.getPosition());

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
		inputDelay = std::max(inputDelay, 0);
		//autoMovement--;
		int posX = (int)player.getPosition().x;
		int posY = (int)player.getPosition().y;


		if (inputDelay <= 0 && moveByInput(player))
			inputDelay = 10;
		
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
			std::cout << "player = " << posX / cellSize << ',' << posY / cellSize<<'\n';

		//ghost update

		std::pair<int, int> source, dest; 
		source.first = (int)Blinky.getPosition().x/ cellSize;
		source.second = (int)Blinky.getPosition().y /cellSize;
		dest.first = (int)player.getPosition().x/ cellSize;
		dest.second = (int)player.getPosition().y/ cellSize;
		
		ghostUpdateRate--;
		if (source != dest && ghostUpdateRate <= 0) {
			ghostUpdateRate = 20;
			//updateBlinky(Blinky, player);
			//updatePinky(Pinky, player, movementDir);
			updateClyde(Clyde, player);
			//for(int i=0;i<3;i++)
			//	std::cout << rand() % 4 << '\n';
		}

		// Food update
		for (unsigned i = 0; i < foods.size(); i++)
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

		for (unsigned i = 0; i < foods.size(); i++)
			window.draw(foods[i]);
		window.draw(Blinky);
		//window.draw(Inky);
		window.draw(Clyde);
		window.draw(Pinky);
		window.draw(player);

		window.display();
	}
	return 0;
}

bool isReachable(char direction, int currentPositionX, int currentPositionY) {  // Returns true if the 4 adjacent cells dont have walls

	int x, y;
	x = currentPositionX / cellSize;
	y = currentPositionY / cellSize;

	if (direction == 'W')
		return notWall[x][y - 1];
	else if (direction == 'S')
		return notWall[x][y + 1];
	else if (direction == 'A')
		return notWall[x - 1][y];
	else if (direction == 'D')
		return notWall[x + 1][y];
	else
		return 0;
};

void moveTowards(sf::Sprite& Entity, char dir) {
	switch (dir){
	case 'W':
		Entity.move(0.f, -1 * cellSize);
		break;	
	case 'S':
		Entity.move(0.f, 1 * cellSize);
		break;	
	case 'A':
		Entity.move(-1 * cellSize, 0.f);
		break;	
	case 'D':
		Entity.move(1 * cellSize, 0.f);
		break;
	}
}


void tracePath(std::pair<int, int> dest)
{
	while (!path.empty())
		path.pop();

	int row, col;
	row = dest.first;
	col = dest.second;
	while (cellDetails[row][col].parent != std::make_pair(row, col))
	{
		path.push(std::make_pair(row, col));
		newRow = cellDetails[row][col].parent.first;
		newCol = cellDetails[row][col].parent.second;
		row = newRow;
		col = newCol;
	}
}

void aStar(std::pair<int, int> source, std::pair<int, int> dest)
{
	if (!openList.empty())
		openList.erase(openList.begin(), openList.end());
	for (int i = 0; i < mapSize + 2; i++)
		for (int j = 0; j < mapSize + 2; j++) {
			cellDetails[i][j].f = cellDetails[i][j].g = cellDetails[i][j].h = INT_MAX;
			cellDetails[i][j].parent.first = cellDetails[i][j].parent.second = -1;
		}

	std::memset(closedList, 0, sizeof(closedList));

	int row, col, newF, newG, newH;
	row = source.first;
	col = source.second;

	openList.insert(std::make_pair(0, std::make_pair(row, col))); // inserting the source in openlist
	cellDetails[row][col].f = cellDetails[row][col].g = cellDetails[row][col].h = 0;
	cellDetails[row][col].parent = source; // The parent of the source is the source itself to make it distinct


	while (!openList.empty())
	{

		parentCell = *openList.begin(); 
		openList.erase(openList.begin());

		row = parentCell.second.first;
		col = parentCell.second.second;

		closedList[row][col] = 1; // inserting parent cell in closedList

		int dx[] = { -1,1,0,0 }, dy[] = { 0,0,1,-1 };

		for (int i = 0; i < 4; i++)
		{
			newRow = row + dx[i];
			newCol = col + dy[i];

			if (!notWall[newRow][newCol] || closedList[newRow][newCol]) //if its a wall or already explored, continue
				continue;

			if (newRow == dest.first && newCol == dest.second) //reached destination
			{
				foundDest = true;
				cellDetails[newRow][newCol].parent = std::make_pair(row, col);
				return;
			}

			newG = cellDetails[row][col].g + 1;
			newH = abs(dest.first - newRow) + abs(dest.second - newCol);
			newF = newG + newH;

			if (newF < cellDetails[newRow][newCol].f || cellDetails[newCol][newRow].f == INT_MAX) 
			{
				openList.insert(std::make_pair(newF, std::make_pair(newRow, newCol)));

				cellDetails[newRow][newCol].f = newF;
				cellDetails[newRow][newCol].g = newG;
				cellDetails[newRow][newCol].h = newH;
				cellDetails[newRow][newCol].parent = std::make_pair(row, col);
			}
		}
		if (foundDest) {
			return;
		}
	}
}

void updateBlinky(sf::Sprite& Blinky, sf::Sprite& player){
	std::pair<int, int> source, dest;
	source.first = (int)Blinky.getPosition().x / cellSize;
	source.second = (int)Blinky.getPosition().y / cellSize;
	dest.first = (int)player.getPosition().x / cellSize;
	dest.second = (int)player.getPosition().y / cellSize;

	foundDest = false;
	aStar(source, dest);
	while (!path.empty())
		path.pop();

	if (foundDest)
	{
		tracePath(dest);
		Blinky.setPosition(path.top().first * cellSize, path.top().second * cellSize);
	}
}
void updatePinky(sf::Sprite& Pinky, sf::Sprite& player, const char& direction){
	std::cout << direction << ' ' ;
	std::pair<int, int> source, dest;
	source.first = (int)Pinky.getPosition().x / cellSize;
	source.second = (int)Pinky.getPosition().y / cellSize;
	dest.first = (int)player.getPosition().x / cellSize;
	dest.second = (int)player.getPosition().y / cellSize;
	
	int dx=0, dy=0;
	char reverseDir;
	switch (direction)
	{
	case 'W': dx = 0, dy = -1;
		break;

	case 'A': dx = -1, dy = 0;
		break;

	case 'S': dx = 0, dy = 1;
		break;

	case 'D': dx = 1, dy = 0;
		break;
	default: dx = dy = 0;
	}
	for (int i = 4; i >= 0; i--)
	{
		newRow = dest.first + dx * i;
		newCol = dest.second + dy * i;
		if (newRow > 38 || newRow < 1 || newCol > 37 || newCol < 1) continue;
		if (notWall[newRow][newCol]) {
			dest.first = newRow;
			dest.second = newCol;
			break;
		}
	}

	//std::cout << dest.first << " " << dest.second << '\n';

	foundDest = false;
	aStar(source, dest);
	while (!path.empty())
		path.pop();

	if (foundDest)
	{
		tracePath(dest);
		Pinky.setPosition(path.top().first * cellSize, path.top().second * cellSize);
	}
}

void updateClyde(sf::Sprite& Clyde, sf::Sprite& player)
{
	int dx, dy;
	std::pair<int,int>source, dest;
	source.first = (int)Clyde.getPosition().x / cellSize;
	source.second = (int)Clyde.getPosition().y / cellSize;
	dest.first = (int)player.getPosition().x / cellSize;
	dest.second = (int)player.getPosition().y / cellSize;
	dx = abs(source.first - dest.first);
	dy = abs(source.second - dest.second);
	foundDest = false;
	if (dx <= 8 && dy <= 8)
		frightenedMove(Clyde);
	else
		aStar(source, dest);
	
	while (!path.empty())
		path.pop();

	if (foundDest)
	{
		tracePath(dest);
		Clyde.setPosition(path.top().first * cellSize, path.top().second * cellSize);
	}
}

bool moveByInput(sf::Sprite& player) {

	int posX, posY;
	posX = player.getPosition().x;
	posY = player.getPosition().y;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) && posX > 0 && isReachable('A', posX, posY))
	{
		movementDir = 'A';
		moveTowards(player, movementDir);
		player.setTexture(playerTextLeft);
		//player.setScale(cellSize / player.getGlobalBounds().width, cellSize / player.getGlobalBounds().height);
		return true;
	}

	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) && posX < cellSize * mapSize - player.getGlobalBounds().width && isReachable('D', posX, posY))
	{
		movementDir = 'D';
		moveTowards(player, movementDir);
		player.setTexture(playerTextRight);
		//player.setScale(cellSize / player.getGlobalBounds().width, cellSize / player.getGlobalBounds().height);
		return true;
	}

	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) && posY > 0 && isReachable('W', posX, posY))
	{
		movementDir = 'W';
		moveTowards(player, movementDir);
		player.setTexture(playerTextUp);
		//player.setScale(cellSize / player.getGlobalBounds().width, cellSize / player.getGlobalBounds().height);
		return true;
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) && posY + player.getGlobalBounds().height < cellSize * mapSize && isReachable('S', posX, posY)) {
		movementDir = 'S';
		moveTowards(player, movementDir);
		player.setTexture(playerTextDown);
		//player.setScale(cellSize / player.getGlobalBounds().width, cellSize / player.getGlobalBounds().height);
		return true;
	}
	player.setTexture(playerTextIdle);
	//player.setScale(cellSize / player.getGlobalBounds().width, cellSize / player.getGlobalBounds().height);
	return false;
}

void frightenedMove(sf::Sprite& Entity)
{
	int dx[] = { -1,1,0,0 }, dy[] = { 0,0,1,-1 }, random = 0;
	std::pair<int, int>source;
	source.first = (int)Entity.getPosition().x/cellSize;
	source.second = (int)Entity.getPosition().y/cellSize;
	int i = 0;		
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> dist(0, 3);
	while (1) {
		random = dist(mt);
		//std::cout << random << '\n';
		newRow = source.first + dx[dist(mt)];
		newCol = source.second + dy[dist(mt)];
		std::cout << "x,y= " << newRow << ',' << newCol << " notwall = " << notWall[newRow][newCol] << '\n';
		if (notWall[newRow][newCol]){
			Entity.setPosition(newRow*cellSize, newCol*cellSize);
			break;
		}
	}
	
}
