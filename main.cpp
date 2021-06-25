#include<SFML/Graphics.hpp>
#include<SFML/Window.hpp>
#include<iostream>
#include<fstream>
#include<cctype>
#include<vector>
#include<string>
#include<set>
#include<stack>
#include<queue>
#include<time.h>
#include<stdlib.h>
#include<cstdio>
#include<random>
#include<chrono>
#include<thread>
#include<Windows.h>

#define cellSize 15   // each cell is (15*15) pixels
#define mapSize 40+1  // a total of 41 rows and columns

class node {
	public:
		std::pair<int,int> parent = std::make_pair(-1,-1);
		char parent2current = '0'; // the last turn taken to reach the current node.
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
sf::Texture BlinkyText, PinkyText, InkyText, ClydeText,FrightenedText,DeadText;
char lastDir[4]; //the direction blinky,pinky,inky,clyde last took 
const int nBlinky = 0, nPinky = 1, nInky = 2, nClyde = 3;
sf::Font font;
bool deadBlinky, deadPinky, deadInky, deadClyde,isFrightened, reduceTime = false;;
std::pair<int, int> home = std::make_pair(16, 19);
int diff = 10, level=1;
int isScattered = 3 * diff, scatterTime = 10, frightenedTime = 20;
std::pair<int, int> scatterPos[4]; // Stores the coordinate where pacman spawns at the start of the round


//functions

bool isReachable(char direction, int currentPositionX, int currentPositionY);   // Returns true if the specified adjacent cells dont have walls
void tracePath(std::pair<int, int> dest,char& dir); // puts the shortest path in a stack named Path
void aStar(std::pair<int, int> source, std::pair<int, int> dest,char& dir); // finds the shortest path from source to dest
void updateBlinky(sf::Sprite& Blinky,sf::Sprite& player);
void updatePinky(sf::Sprite& Pinky, sf::Sprite& player); //direction is the way player is moving
void updateClyde(sf::Sprite& Clyde, sf::Sprite& player);
void updateInky(sf::Sprite& Inky, sf::Sprite& Blinky, sf::Sprite& player);
void moveTowards(sf::Sprite& Entity, char dir); // player/ghost moves toward said direction
bool moveByInput(sf::Sprite& player);
void frightenedMove(sf::Sprite& Entity,char& dir);
char reverseDir(const char& dir);
bool isWithinMap(std::pair<int, int>& node);
bool scatter(sf::Sprite& Entity, char& dir,std::pair<int,int>& const scatterPos);
void game_over(unsigned& score,unsigned& highScore,sf::RenderWindow& window);
void nextRound(sf::RenderWindow& window,int& level,std::vector < sf::Sprite >& foods,sf::Sprite& food);
bool isGhostOnPlayer(sf::Sprite& player, sf::Sprite& Blinky, sf::Sprite& Inky, sf::Sprite& Pinky, sf::Sprite& Clyde);
void setFrightenedText(sf::Sprite& Blinky, sf::Sprite& Inky, sf::Sprite& Pinky, sf::Sprite& Clyde);
void removeFrightenedText(sf::Sprite& Blinky, sf::Sprite& Inky, sf::Sprite& Pinky, sf::Sprite& Clyde);

int main() {

	sf::RenderWindow window(sf::VideoMode(cellSize*mapSize, cellSize*mapSize), "Pacman");
	window.setFramerateLimit(60);

	std::memset(lastDir, '0', sizeof(lastDir));
	int inputDelay = 20, ghostUpdateRate = 35;
	bool isGameOver = false;
	srand(time(0));

	//food

	sf::Sprite food;
	sf::Texture foodText;
	if (!foodText.loadFromFile("Resources/Texture/food.png"))
		std::cout << "failed to load food.png";
	food.setTexture(foodText);
	food.setScale(cellSize / food.getGlobalBounds().width , cellSize / food.getGlobalBounds().height);
	std::vector < sf::Sprite > foods;


	//power up

	sf::Sprite powerUp;
	sf::Texture powerUpText;
	if (!powerUpText.loadFromFile("Resources/Texture/power_pellet.png"))
		std::cout << "failed to load powerpellet.png";
	powerUp.setTexture(powerUpText);
	powerUp.setScale(cellSize / powerUp.getGlobalBounds().width, cellSize / powerUp.getGlobalBounds().height);
	std::vector <sf::Sprite > powerUps;
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
	srand(time(0));
	if(mapInfo.is_open())
	{
		std::string row;
		int pos = 0,j=0;
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
					scatterPos[j].first = pos * cellSize; 
					scatterPos[j++].second = i * cellSize;
					map[pos][i].setFillColor(sf::Color::Blue);//debug
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
	for (int i = 0; i < 4; i++)
	{
		powerUps.push_back(powerUp);
		powerUps[powerUps.size() - 1].setPosition(scatterPos[i].first, scatterPos[i].second);
	}


	//fonts

	if (!font.loadFromFile("Resources/Fonts/Noto_Sans/NotoSans-Regular.ttf"))
		std::cout << "error loading fonts" << '\n';
	
	//texts and fonts
	sf::Text statsText;
	std::string stats;
	statsText.setFont(font);
	
	//initializing strings and setting them to their corresponding texts
	stats = { "Score : " }; 
	statsText.setString(stats);
	statsText.setCharacterSize(1.5*cellSize);
	statsText.setPosition(16 * cellSize, 2 * cellSize);
	statsText.setFillColor(sf::Color::Blue);
	statsText.setStyle(sf::Text::Bold);


	//scores
	unsigned nScore =0 , nHighScore , nHp = 3;
	std::fstream scoreFile;
	scoreFile.open("high_score.txt",std::ios::in || std::ios::out);
	if (scoreFile.is_open())
		scoreFile >> nHighScore;

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
	player.setPosition(19 * cellSize , 19*cellSize);


	// ghost

	isFrightened = deadBlinky = deadPinky = deadInky = deadClyde = false;

	if (!FrightenedText.loadFromFile("Resources/Texture/frightened.png"))
		std::cout << "failed to load frightened.png";	
	if (!DeadText.loadFromFile("Resources/Texture/dead_ghost.png"))
		std::cout << "failed to load dead_ghost.png";


	sf::Sprite Blinky, Pinky, Inky, Clyde;
	if (!BlinkyText.loadFromFile("Resources/Texture/blinkyDown.png"))
		std::cout << "failed to load blinky.png";
	Blinky.setTexture(BlinkyText);
	Blinky.setScale(cellSize/Blinky.getGlobalBounds().width,cellSize/Blinky.getGlobalBounds().height);
	Blinky.setPosition(scatterPos[nBlinky].first, scatterPos[nBlinky].second);

	
	if (!ClydeText.loadFromFile("Resources/Texture/clydeDown.png"))
		std::cout << "failed to load clyde.png";
	Clyde.setTexture(ClydeText);
	Clyde.setScale(cellSize/Clyde.getGlobalBounds().width,cellSize/Clyde.getGlobalBounds().height );
	//std::cout << "clyde = " << Clyde.getGlobalBounds().height << " " << Clyde.getGlobalBounds().width << '\n';
	Clyde.setPosition(scatterPos[nClyde].first, scatterPos[nClyde].second);

	if (!InkyText.loadFromFile("Resources/Texture/inkyDown.png"))
		std::cout << "failed to load inky.png";

	//Inky.setTexture(FrightenedText);
	Inky.setTexture(InkyText);
	Inky.setScale(cellSize/Inky.getGlobalBounds().width ,cellSize/Inky.getGlobalBounds().height );
	
	std::cout << "inky= " << Inky.getGlobalBounds().height << " " << Inky.getGlobalBounds().width << '\n';
	Inky.setPosition(scatterPos[nInky].first, scatterPos[nInky].second);
	
	if (!PinkyText.loadFromFile("Resources/Texture/pinkyDown.png"))
		std::cout << "failed to laod pinky.png";
	Pinky.setTexture(PinkyText);
	Pinky.setScale(cellSize / Pinky.getGlobalBounds().width, cellSize / Pinky.getGlobalBounds().height);
	Pinky.setPosition(scatterPos[nPinky].first, scatterPos[nPinky].second);



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
		if (inputDelay <= 0 && moveByInput(player))
			inputDelay = 10;
		
		
		//ghost update

		ghostUpdateRate--;
		
		if (isFrightened && ghostUpdateRate <= 0)
		{
			std::cout << "SCARY" << '\n';
			ghostUpdateRate = std::max(40 - 2 * level, 25);
			frightenedTime--;
			if(!deadBlinky)
				frightenedMove(Blinky, lastDir[nBlinky]);
			else 


			if(!deadPinky)
				frightenedMove(Pinky, lastDir[nPinky]);
			if(!deadInky)
				frightenedMove(Inky, lastDir[nInky]);
			if (!deadClyde)
				frightenedMove(Clyde, lastDir[nClyde]);

			if (frightenedTime <= 0)
			{
				isFrightened = false;
				removeFrightenedText(Blinky, Inky, Pinky, Clyde);
				frightenedTime = 20;
			}
		}


		else if (isScattered <= 0 && ghostUpdateRate <= 0)
		{
			ghostUpdateRate = std::max(40 - 2 * level,25);
			if (scatterTime <= 0)
			{
				isScattered = std::min(100, level * 30);
				scatterTime = 10;
			}
			else {
				
				


				//if one of the ghost reached their dest, scattertime will start counting
				if (scatter(Blinky, lastDir[nBlinky], scatterPos[nBlinky]))
					reduceTime = true;
				if (scatter(Pinky, lastDir[nPinky], scatterPos[nPinky]))
				reduceTime = true;
				if (scatter(Clyde, lastDir[nClyde], scatterPos[nClyde]))
				reduceTime = true;
				if (scatter(Inky, lastDir[nInky], scatterPos[nInky]))
				reduceTime = true;

				if (reduceTime)
					scatterTime--;
			}
		}

		else if (ghostUpdateRate <= 0) {
			isScattered--;
			ghostUpdateRate = std::max(40 - 2 * level, 25);
			updateBlinky(Blinky, player);
			updatePinky(Pinky, player);
			updateClyde(Clyde, player);
			updateInky(Inky, Blinky, player);
		}

		// collisions


		if (isGhostOnPlayer(player, Blinky, Inky, Pinky, Clyde) && isFrightened)
		{

			if (player.getGlobalBounds().intersects(Blinky.getGlobalBounds()))
			{
				Blinky.setTexture(DeadText);
				Blinky.setScale(cellSize / Blinky.getGlobalBounds().width, cellSize / Blinky.getGlobalBounds().height);
				deadBlinky = true;
			}
			if (player.getGlobalBounds().intersects(Inky.getGlobalBounds()))
			{
				Inky.setTexture(DeadText);
				Inky.setScale(cellSize / Inky.getGlobalBounds().width, cellSize / Inky.getGlobalBounds().height);
				deadInky = true;
			}
			if (player.getGlobalBounds().intersects(Pinky.getGlobalBounds()))
			{
				Pinky.setTexture(DeadText);
				Pinky.setScale(cellSize / Pinky.getGlobalBounds().width, cellSize / Pinky.getGlobalBounds().height);
				deadPinky = true;
			}
			if (player.getGlobalBounds().intersects(Clyde.getGlobalBounds()))
			{
				Clyde.setTexture(DeadText);
				Clyde.setScale(cellSize / Clyde.getGlobalBounds().width, cellSize / Clyde.getGlobalBounds().height);
				deadClyde = true;
			}
		}
		else if (isGhostOnPlayer(player, Blinky, Inky, Pinky, Clyde) && !isFrightened)
		{
			nHp--;
			if (nHp <= 0)
			{
				game_over(nScore, nHighScore, window);
				break;
			}
			player.setPosition(19 * cellSize, 19 * cellSize);
			Blinky.setPosition(scatterPos[nBlinky].first, scatterPos[nBlinky].second);
			Inky.setPosition(scatterPos[nInky].first, scatterPos[nInky].second);
			Clyde.setPosition(scatterPos[nClyde].first, scatterPos[nClyde].second);
			Pinky.setPosition(scatterPos[nPinky].first, scatterPos[nPinky].second);
		}



		//powerpelllet
		for (unsigned i = 0; i < powerUps.size(); i++)
			if (powerUps[i].getGlobalBounds().intersects(player.getGlobalBounds()))
			{
				powerUps.erase(powerUps.begin() + i);
				isFrightened = true;
				setFrightenedText(Blinky, Inky, Pinky, Clyde);
			}



		// Food update
		for (unsigned i = 0; i < foods.size(); i++)
			if (foods[i].getGlobalBounds().intersects(player.getGlobalBounds()))
			{
				foods.erase(foods.begin() + i);
				nScore += 10;
			}


		//score update
		stats.clear();
		stats = { "Score = " };
		stats += std::to_string(nScore);
		stats += {"\nHp = "};
		stats += std::to_string(nHp);
		statsText.setString(stats);
		
		
		window.clear(sf::Color::Transparent);

		if (foods.empty()) {
			nextRound(window, ++level, foods, food);
			player.setPosition(19 * cellSize, 19 * cellSize);
			Blinky.setPosition(scatterPos[nBlinky].first, scatterPos[nBlinky].second);
			Inky.setPosition(scatterPos[nInky].first, scatterPos[nInky].second);
			Clyde.setPosition(scatterPos[nClyde].first, scatterPos[nClyde].second);
			Pinky.setPosition(scatterPos[nPinky].first, scatterPos[nPinky].second);
		}

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
		for (int i = 0; i < powerUps.size(); i++)
			window.draw(powerUps[i]);

		window.draw(Blinky);
		window.draw(Inky);
		window.draw(Clyde);
		window.draw(Pinky);
		window.draw(player);
		window.draw(statsText);

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


void tracePath(std::pair<int, int> dest,char& dir)
{
	while (!path.empty())
		path.pop();

	int row, col;
	row = dest.first;
	col = dest.second;
	//std::cout << "cant go at " << reverseDir(dir) << '\n';
	while (cellDetails[row][col].parent != std::make_pair(row, col))
	{
		dir = cellDetails[row][col].parent2current;
		path.push(std::make_pair(row, col));
		newRow = cellDetails[row][col].parent.first;
		newCol = cellDetails[row][col].parent.second;
		row = newRow;
		col = newCol;
	}
	//std::cout<<"going at " << dir << '\n';
}
char reverseDir(const char& dir) {
	switch (dir)
	{
	case 'W': return 'S';

	case 'A': return 'D';

	case 'S': return 'W';

	case 'D': return 'A';

	default: return '0';
	}
}
bool isWithinMap(std::pair<int, int>& node)
{
	bool within = true;
	if (node.first > 38) {
		node.first = 38;
		within = false;
	}
	if (node.second > 37) {
		node.second = 37;
		within = false;
	}
	if (node.first < 1) {
		node.first = 1;
		within = false;
	}
	if (node.second < 1)
	{
		node.second = 1;
		within = false;
	}
	return within;
}
bool scatter(sf::Sprite& Entity, char& dir,std::pair<int,int>& const scatterPos)
{
	std::pair<int, int> source,dest;
	source.first = (int)Entity.getPosition().x / cellSize;
	source.second = (int)Entity.getPosition().y / cellSize;
	dest.first = scatterPos.first / cellSize;
	dest.second = scatterPos.second / cellSize;
	while (!path.empty())
		path.pop();

	if (dest == source) {
		//std::cout << "TRUE"<<'\n';
		return true;
	}
	foundDest = false;
	aStar(source, dest, dir);
	if (foundDest)
	{
		tracePath(dest, dir);
		Entity.setPosition(path.top().first * cellSize, path.top().second * cellSize);
	}
	return false;
}
void aStar(std::pair<int, int> source, std::pair<int, int> dest ,char& dir)
{
	if (!openList.empty())
		openList.erase(openList.begin(), openList.end());
	for (int i = 0; i < mapSize + 2; i++)
		for (int j = 0; j < mapSize + 2; j++) {
			cellDetails[i][j].f = cellDetails[i][j].g = cellDetails[i][j].h = INT_MAX;
			cellDetails[i][j].parent.first = cellDetails[i][j].parent.second = -1;
			cellDetails[i][j].parent2current = '0';//add
		}

	std::memset(closedList, 0, sizeof(closedList));

	int row, col, newF, newG, newH;
	row = source.first;
	col = source.second;

	openList.insert(std::make_pair(0, std::make_pair(row, col))); // inserting the source in openlist
	cellDetails[row][col].f = cellDetails[row][col].g = cellDetails[row][col].h = 0;
	cellDetails[row][col].parent = source; // The parent of the source is the source itself to make it distinct
	cellDetails[row][col].parent2current = dir; // The turn the ghost took to reach source


	while (!openList.empty())
	{

		parentCell = *openList.begin(); 
		openList.erase(openList.begin());

		row = parentCell.second.first;
		col = parentCell.second.second;

		closedList[row][col] = 1; // inserting parent cell in closedList

		int dx[] = { -1,1,0,0 }, dy[] = { 0,0,1,-1 }; //left,right,down,up

		for (int i = 0; i < 4; i++)
		{
			if (i == 0 && cellDetails[row][col].parent2current =='D')continue;
			if (i == 1 && cellDetails[row][col].parent2current == 'A')continue;
			if (i == 2 && cellDetails[row][col].parent2current == 'W')continue;
			if (i == 3 && cellDetails[row][col].parent2current == 'S')continue;

			newRow = row + dx[i];
			newCol = col + dy[i];

			if (!notWall[newRow][newCol] || closedList[newRow][newCol]) //if its a wall or already explored, continue
				continue;

			if (newRow == dest.first && newCol == dest.second) //reached destination
			{
				foundDest = true;
				cellDetails[newRow][newCol].parent = std::make_pair(row, col);
				if (i == 0)dir = 'A';
				else if (i ==1 )dir = 'D';
				else if (i == 2)dir = 'S';
				else if (i == 3)dir = 'W';
				return;
			}

			newG = cellDetails[row][col].g + 1;
			newH = abs(dest.first - newRow) + abs(dest.second - newCol);
			newF = newG + newH;
			int diff = abs(cellDetails[newRow][newCol].parent.first - source.first) + abs(cellDetails[newRow][newCol].parent.second - source.second);
			/*if (diff==1 && cellDetails[newRow][newCol].parent2current == reverseDir(dir))
				continue;*/
			if (newF < cellDetails[newRow][newCol].f || cellDetails[newCol][newRow].f == INT_MAX) 
			{
				openList.insert(std::make_pair(newF, std::make_pair(newRow, newCol)));

				cellDetails[newRow][newCol].f = newF;
				cellDetails[newRow][newCol].g = newG;
				cellDetails[newRow][newCol].h = newH;
				cellDetails[newRow][newCol].parent = std::make_pair(row, col);
				if (i == 0)cellDetails[newRow][newCol].parent2current = 'A';
				else if (i == 1)cellDetails[newRow][newCol].parent2current = 'D';
				else if (i == 2)cellDetails[newRow][newCol].parent2current = 'S';
				else if (i == 3)cellDetails[newRow][newCol].parent2current = 'W';

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
	while (!path.empty())
		path.pop();
	foundDest = false;
	if (deadBlinky)
	{
		dest = home;
		aStar(source, dest, lastDir[nBlinky]);
	}
	else if (isFrightened) {
		frightenedMove(Blinky, lastDir[nBlinky]);
		return;
	}
	else if (isScattered <= 0)
	{
		if (scatterTime <= 0)
		{
			isScattered = std::min(100, level * 30);
			scatterTime = 10;
		}
		if(scatter(Blinky,lastDir[nBlinky],scatterPos[nBlinky]))
			reduceTime = true;
	}

	
	aStar(source, dest,lastDir[nBlinky]);

	if (foundDest)
	{	
		//std::cout << "Came From = " << source.first << ',' << source.second << '\n';
		tracePath(dest,lastDir[nBlinky]);
		Blinky.setPosition(path.top().first * cellSize, path.top().second * cellSize);
		//std::cout << "Currently at = " << path.top().first << ',' << path.top().second << '\n';
		//std::cout << "dir = " << lastDir[nBlinky] << '\n';
	}
}
void updatePinky(sf::Sprite& Pinky, sf::Sprite& player){
	//std::cout << movementDir << ' ' ;
	std::pair<int, int> source, dest,temp;
	source.first = (int)Pinky.getPosition().x / cellSize;
	source.second = (int)Pinky.getPosition().y / cellSize;
	dest.first = (int)player.getPosition().x / cellSize;
	dest.second = (int)player.getPosition().y / cellSize;
	while (!path.empty())
		path.pop();
	
	int dx=0, dy=0;
	switch (movementDir)
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
		temp.first = dest.first + dx * i;
		temp.second = dest.second + dy * i;
		if (!isWithinMap(temp))
			continue;
		newRow = temp.first;
		newCol = temp.second;
		if (notWall[newRow][newCol]) {
			dest.first = newRow;
			dest.second = newCol;
			break;
		}
	}

	//std::cout << dest.first << " " << dest.second << '\n';

	foundDest = false;
	aStar(source, dest,lastDir[nPinky]);

	if (foundDest)
	{
		tracePath(dest,lastDir[nPinky]);
		Pinky.setPosition(path.top().first * cellSize, path.top().second * cellSize);
	}
}

void updateClyde(sf::Sprite& Clyde, sf::Sprite& player)
{
	int dx, dy;
	std::pair<int, int>source, dest;

	source.first = (int)Clyde.getPosition().x / cellSize;
	source.second = (int)Clyde.getPosition().y / cellSize;

	dest.first = (int)player.getPosition().x / cellSize;
	dest.second = (int)player.getPosition().y / cellSize;

	dx = abs(source.first - dest.first);
	dy = abs(source.second - dest.second);

	if (dx <= 8 && dy <= 8)
		frightenedMove(Clyde, lastDir[nClyde]);
	else
	{
		while (!path.empty())
			path.pop();
		foundDest = false;
		aStar(source, dest, lastDir[nClyde]);
		if (foundDest)
		{
			tracePath(dest, lastDir[nClyde]);
			Clyde.setPosition(path.top().first * cellSize, path.top().second * cellSize);
		}
	}
}

void updateInky(sf::Sprite& Inky, sf::Sprite& Blinky, sf::Sprite& player)
{
	std::pair<int, int>source, dest, posBlinky,temp;
	source.first = Inky.getPosition().x / cellSize;
	source.second = Inky.getPosition().y / cellSize;
	dest.first = player.getPosition().x / cellSize;
	dest.second = player.getPosition().y / cellSize;
	posBlinky.first = Blinky.getPosition().x / cellSize;
	posBlinky.second = Blinky.getPosition().y / cellSize;
	while (!path.empty())
		path.pop();

	int dx = 0, dy = 0;
	switch (movementDir)
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
	for (int i = 2; i >= 0; i--)
	{
		temp.first = dest.first + dx * i;
		temp.second = dest.second + dy * i;
		if (!isWithinMap(temp)) continue;
		newRow = temp.first;
		newCol = temp.second;
		if (notWall[newRow][newCol]) {
			dest.first = newRow;
			dest.second = newCol;
			break;
		}
	}

	dx = (dest.first > posBlinky.first ) ? 1 : -1;
	if (posBlinky.first == dest.first)
		dx = 0;
	dy = (dest.second > posBlinky.second) ? 1 : -1;
	if (posBlinky.second == dest.second)
		dy = 0;

	dest.first += dx * 4;
	dest.second += dy * 4;
	//dest.first += dest.first - posBlinky.first;
	//dest.second += dest.second - posBlinky.second;
	////std::cout << "perfect dest =" << dest.first << ',' << dest.second << '\n';

	newRow = player.getPosition().x / cellSize;
	newCol = player.getPosition().y / cellSize;
	isWithinMap(dest);
		//std::cout << "wasnt perfect dest =" << dest.first << ',' << dest.second << '\n';

	while (!notWall[dest.first][dest.second])
	{
		dx = (newRow > dest.first) ? 1 : -1;
		if (newRow == dest.first)
			dx = 0;
		dy = (newCol > dest.second) ? 1 : -1;
		if (newCol == dest.second)
			dy = 0;

		dest.first += dx;
		dest.second += dy;
	}
	//std::cout <<"final dest =" << dest.first << ',' << dest.second <<"---"<<notWall[dest.first][dest.second]<<'\n';
	foundDest = false;
	aStar(source, dest, lastDir[nInky]);

	if (foundDest)
	{
		tracePath(dest, lastDir[nInky]);
		//std::cout << lastDir[nInky] << '\n';
		Inky.setPosition(path.top().first * cellSize, path.top().second * cellSize);
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
		return true;
	}

	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) && posX < cellSize * mapSize - player.getGlobalBounds().width && isReachable('D', posX, posY))
	{
		movementDir = 'D';
		moveTowards(player, movementDir);
		player.setTexture(playerTextRight);
		return true;
	}

	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) && posY > 0 && isReachable('W', posX, posY))
	{
		movementDir = 'W';
		moveTowards(player, movementDir);
		player.setTexture(playerTextUp);
		return true;
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) && posY + player.getGlobalBounds().height < cellSize * mapSize && isReachable('S', posX, posY)) {
		movementDir = 'S';
		moveTowards(player, movementDir);
		player.setTexture(playerTextDown);
		return true;
	}
	player.setTexture(playerTextIdle);
	return false;
}

void frightenedMove(sf::Sprite& Entity,char& dir)
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
		if (random == 0 && dir == 'D')continue;
		if (random == 1 && dir == 'A')continue;
		if (random == 2 && dir == 'W')continue;
		if (random == 3 && dir == 'S')continue;

		newRow = source.first + dx[random];
		newCol = source.second + dy[random];

		if (notWall[newRow][newCol]){

			Entity.setPosition(newRow*cellSize, newCol*cellSize);
			if (random == 0)dir = 'A';
			else if (random == 1)dir = 'D';
			else if (random == 2)dir = 'S';
			else if (random == 3)dir = 'W';
			break;
		}
	}
	
}

void game_over(unsigned& score,unsigned& highScore,sf::RenderWindow& window) {
	int x, y;
	sf::Text overText;
	overText.setFont(font);
	overText.setStyle(sf::Text::Bold);
	std::string _over;
	if (score >= highScore)
	{
		std::fstream file;
		file.open("high_score.txt", std::fstream::out | std::fstream::trunc);
		//if (file.is_open())
			//std::cout << "Open" << '\n';

		file << std::to_string(score);
		file.close();

		_over = { "Congratulations!\nNew High Score!\nScore = " };
		_over += std::to_string(score);
		overText.setString(_over);
		overText.setCharacterSize(cellSize);
		x = (window.getSize().x) /2 - overText.getGlobalBounds().width/2;
		y = window.getSize().y / 2 -overText.getGlobalBounds().height/2;
		//std::cout << "x,y=" << x << ',' << y << '\n';
		overText.setPosition(x, y);
		overText.setFillColor(sf::Color::Red);
		window.clear(sf::Color::Transparent);
		window.draw(overText);
		window.display();
		std::chrono::seconds dura(3);
		std::this_thread::sleep_for(dura);
		file.close();
	}
	else {
		_over = { "Game Over!\nScore = " };
		_over += std::to_string(score);
		_over += {"\nHigh Score = "};
		_over += std::to_string(highScore);
		overText.setCharacterSize(cellSize);
		x = window.getSize().x / 2 - overText.getGlobalBounds().width / 2;
		y = window.getSize().y / 2 - overText.getGlobalBounds().height / 2;
		overText.setString(_over);
		overText.setPosition(x, y);
		overText.setFillColor(sf::Color::Red);

		window.clear(sf::Color::Transparent);
		
		window.draw(overText);
		window.display();    
		std::chrono::seconds dura(3);
		std::this_thread::sleep_for(dura);
	}




}

void nextRound(sf::RenderWindow& window, int& level, std::vector < sf::Sprite >& foods, sf::Sprite& food)
{
	window.clear(sf::Color::Transparent);
	sf::Text roundText;
	std::string round = {"Starting Round "};
	round += std::to_string(level);
	round += {"..."};
	roundText.setString(round);
	roundText.setFillColor(sf::Color::Blue);
	roundText.setFont(font);
	roundText.setCharacterSize(cellSize);
	int x, y;
	x = window.getSize().x / 2 - roundText.getGlobalBounds().width/2;
	y = window.getSize().y / 2 - roundText.getGlobalBounds().height/2;
	roundText.setPosition(x, y);
	srand(time(0));
	window.draw(roundText);
	window.display();
	Sleep(3000);
	window.clear(sf::Color::Transparent);


	for (int i = 0; i < 40; i++)
		for(int j=0;j< 40;j++)
			if (notWall[i][j] && rand()%6==1) // if there is a wall
			{
				foods.push_back(food);
				foods[foods.size() - 1].setPosition((i * cellSize), (j * cellSize));
			}
}

bool isGhostOnPlayer(sf::Sprite& player, sf::Sprite& Blinky, sf::Sprite& Inky, sf::Sprite& Pinky, sf::Sprite& Clyde)
{
	if (player.getGlobalBounds().intersects(Blinky.getGlobalBounds()))
		return true;
	if (player.getGlobalBounds().intersects(Inky.getGlobalBounds()))
		return true;
	if (player.getGlobalBounds().intersects(Pinky.getGlobalBounds()))
		return true;
	if (player.getGlobalBounds().intersects(Clyde.getGlobalBounds()))
		return true;
	return false;
}

void setFrightenedText(sf::Sprite& Blinky, sf::Sprite& Inky, sf::Sprite& Pinky, sf::Sprite& Clyde)
{
	Inky.setTexture(FrightenedText);
	Clyde.setTexture(FrightenedText);
	Blinky.setTexture(FrightenedText);
	Pinky.setTexture(FrightenedText);
}

void removeFrightenedText(sf::Sprite& Blinky, sf::Sprite& Inky, sf::Sprite& Pinky, sf::Sprite& Clyde)
{
	Blinky.setTexture(BlinkyText);
	Pinky.setTexture(PinkyText);
	Inky.setTexture(InkyText);
	Clyde.setTexture(ClydeText); 
	//Blinky.setScale(cellSize / Blinky.getGlobalBounds().width, cellSize / Blinky.getGlobalBounds().height);
	//Pinky.setScale(cellSize / Pinky.getGlobalBounds().width, cellSize / Pinky.getGlobalBounds().height);
	//Inky.setScale(cellSize / Inky.getGlobalBounds().width, cellSize / Inky.getGlobalBounds().height);
	//Clyde.setScale(cellSize / Clyde.getGlobalBounds().width, cellSize / Clyde.getGlobalBounds().height);

}
