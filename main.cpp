#include<SFML/Graphics.hpp>
#include<SFML/Window.hpp>
#include<iostream>
#include<fstream>
#include<cctype>
#include<vector>
#include<string>
#include<set>
#include<stack>

#define cellSize 15   // each cell is (15*15) pixels
#define mapSize 40+1  // a total of 41 rows and columns

class node {
	public:
		std::pair<int,int> parent;
		int f=INT_MAX, g = INT_MAX, h = INT_MAX;// f = estimated length of source to destination on a path = g+h . g = source to current node. h = estimated distance between current node and dest
	//bool operator < (const node& rhs) const { return f < rhs.f; }
};

//Some global variables frequently used by functions
bool foundDest = false;
bool notWall[mapSize+2][mapSize+2];
node cellDetails[mapSize + 2][mapSize + 2];
bool closedList[mapSize + 2][mapSize + 2]; // contains nodes that have been fully explored
std::set <std::pair<int, std::pair<int, int> > > openList; // Contains f and the coordinate of a cell
std::stack< std::pair<int, int> > path;


bool isReachable(bool grid[][43], char direction, int currentPositionX, int currentPositionY ) {  // Returns true if the 4 adjacent cells dont have walls
	
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


void tracePath(std::pair<int, int> dest, node cellDetails[][mapSize])
{
	while (!path.empty())
		path.pop();

	int row, col;
	row = dest.first;
	col = dest.second;
	while (cellDetails[row][col].parent != std::make_pair(row, col)) { // float comparison
		path.push(std::make_pair(row, col));
		int newRow = cellDetails[row][col].parent.first;
		int newCol = cellDetails[row][col].parent.second;
		row = newRow;
		col = newCol;
	}	
}

void aStar(std::pair<int,int> source, std::pair<int,int> dest)
{
	for (int i = 0; i < mapSize + 2; i++)
		for (int j = 0; j < mapSize + 2; j++)
		{
			cellDetails[i][j].f = cellDetails[i][j].g = cellDetails[i][j].h = INT_MAX;
			cellDetails[i][j].parent.first = cellDetails[i][j].parent.second = -1;
		}

	std::memset(closedList, 0, sizeof(closedList));
	//for (int i = 0; i < mapSize + 2; i++)
	//	for (int j = 0; j < mapSize + 2; j++)
	//		closedList[i][j] = 0;

	int row, col; // ghost location
	row = source.first;
	col = source.second;

	openList.insert(std::make_pair(0, std::make_pair(row, col))); // inserting the source in openlist
	cellDetails[row][col].f = cellDetails[row][col].g = cellDetails[row][col].h = 0;
	cellDetails[row][col].parent = source; // The parent of the source is the source itself to make it distinct


	while (!openList.empty())
	{
		std::pair<int, std::pair<int, int> > parentCell = *openList.begin();
		openList.erase(openList.begin());

		row = parentCell.second.first;
		col = parentCell.second.second;

		closedList[row][col] = 1; // inserting parent cell in closedList

		int dx[] = { -1,1,0,0 }, dy[] = { 0,0,1,-1 };

		for (int i = 0; i < 4; i++)
		{
			int newRow, newCol;
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

			int newF, newG, newH;
			newG = cellDetails[row][col].g + 1;
			newH = abs(dest.first - newRow) + abs(dest.second - newCol);
			newF = newG + newH;

			if (newF < cellDetails[newRow][newCol].f || cellDetails[newCol][newRow].f == INT_MAX) // slight change 
			{
				openList.insert(std::make_pair(newF, std::make_pair(newRow, newCol)));

				cellDetails[newRow][newCol].f = newF;
				cellDetails[newRow][newCol].g = newG;
				cellDetails[newRow][newCol].h = newH;
				cellDetails[newRow][newCol].parent = std::make_pair(row, col);
			}
		}
		if (foundDest) return;
	}
}


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
	player.setPosition(pacmanSpawn.first,pacmanSpawn.second);

	std::cout << pacmanSpawn.first << pacmanSpawn.second << '\n';

	std::cout << "Works! " << player.getGlobalBounds().height << " " << player.getGlobalBounds().width;

	// ghost
	sf::Sprite Blinky, Pinky, Inky, Clyde;
	sf::Texture BlinkyText, PinkyText, InkyText, ClydeText;
	if (!BlinkyText.loadFromFile("Resources/Texture/blinky.png"))
		std::cout << "failed to load blinky.png";
	Blinky.setTexture(BlinkyText);
	Blinky.setScale(cellSize/Blinky.getGlobalBounds().width,cellSize/Blinky.getGlobalBounds().height);
	Blinky.setPosition(pacmanSpawn.first,pacmanSpawn.second);
	std::cout << "blinky= " << Blinky.getGlobalBounds().height << " " << Blinky.getGlobalBounds().width << '\n';

	if (!ClydeText.loadFromFile("Resources/Texture/clyde.png"))
		std::cout << "failed to load clyde.png";
	Clyde.setTexture(ClydeText);
	Clyde.setScale(cellSize/Clyde.getGlobalBounds().width,cellSize/Clyde.getGlobalBounds().height );
	std::cout << "clyde = " << Clyde.getGlobalBounds().height << " " << Clyde.getGlobalBounds().width << '\n';
	Clyde.setPosition(sf::Vector2f(window.getSize().x - Clyde.getGlobalBounds().width, 0.f));

	if (!InkyText.loadFromFile("Resources/Texture/inky.png"))
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
		int posX = (int)player.getPosition().x;
		int posY = (int)player.getPosition().y;
		int updateBlinky;
		updateBlinky = std::max(inputDelay,autoMovement);
		
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
		//ghost update

		//std::stack<sf::Vector2f> printPath;
		//printPath = path;
		//if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
		//{
		//	std::cout << "space pressed" << std::endl;
		//	while (!path.empty())
		//	{
		//		map[(int)path.top().x][(int)path.top().y].setFillColor(sf::Color::Red);
		//		path.pop();
		//	}
		//}

		std::pair<int, int> source, dest; 
		source.first = (int)Blinky.getPosition().x/ cellSize;
		source.second = (int)Blinky.getPosition().y /cellSize;
		dest.first = (int)player.getPosition().x/ cellSize;
		dest.second = (int)player.getPosition().y/ cellSize;


		
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad0) && updateBlinky<=0)
			std::cout <<"source = " << source.first << " - " << source.second << '\n';
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Numpad1) && updateBlinky<=0)
			std::cout <<"dest = " << dest.first << " - " << dest.second << '\n';

		dest = std::make_pair(555/cellSize, 315/cellSize); //debug

		updateBlinky = 20;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))updateBlinky = 0;

		if (source != dest && updateBlinky<=0)
		{
			foundDest = false;
			aStar(source, dest);

			//for (int i = 0; i < mapSize+2; i++)
			//	for (int j = 0; j < mapSize+2; j++)
			//	{
			//		cellDetails[i][j].f = cellDetails[i][j].g = cellDetails[i][j].h = INT_MAX;
			//		cellDetails[i][j].parent.first = cellDetails[i][j].parent.second = -1;
			//	}

			////std::memset(closedList, 0, sizeof(closedList));
			//for (int i = 0; i < mapSize + 2; i++)
			//	for (int j = 0; j < mapSize + 2; j++)
			//		closedList[i][j] = 0;

			//int row, col; // ghost location
			//row = source.first;
			//col = source.second;

			////closedList[row][col] = 1;
			//openList.insert(std::make_pair(0, std::make_pair(row, col))); // inserting the source in openlist
			//cellDetails[row][col].f = cellDetails[row][col].g = cellDetails[row][col].h = 0;
			//cellDetails[row][col].parent = source; // The parent of the source is the source itself to make it distinct
			//

			//while (!openList.empty())
			//{

			//	//std::set<int, std::pair<int,int> >::iterator itr;
			//	//for (itr = openList.begin(); itr != openList.size(); itr++)
			//	//	std::cout << "f =" << *itr.first << " ->" << *itr.second.first << " , " << *itr.second << '\n';
			//	for (const auto a : openList)
			//	{
			//		std::cout << "f =" << a.first << " ->" << a.second.first << ',' << a.second.second << " 	|	";
			//	}
			//	std::cout << '\n';
			//	std::pair<int, std::pair<int, int> > parentCell = *openList.begin();
			//	openList.erase(openList.begin());

			//	row = parentCell.second.first;
			//	col = parentCell.second.second;

			//	closedList[row][col] = 1; // inserting parent cell in closedList

			//	int dx[] = { -1,1,0,0 }, dy[] = { 0,0,1,-1 };

			//	for (int i = 0; i < 4; i++)
			//	{
			//		int newRow, newCol;
			//		newRow = row + dx[i];
			//		newCol = col + dy[i];

			//		if (!notWall[newRow][newCol] || closedList[newRow][newCol]) //if its a wall or already explored, continue
			//			continue;

			//		if (newRow == dest.first && newCol == dest.second) //reached destination
			//		{
			//			foundDest = true;
			//			cellDetails[newRow][newCol].parent = std::make_pair(row,col); //cgabge
			//			break; // return changed to break
			//		}

			//		int newF, newG, newH;
			//		newG = cellDetails[row][col].g + 1;
			//		newH = abs(dest.first - newRow) + abs(dest.second - newCol);
			//		newF = newG + newH;

			//		if (newF < cellDetails[newRow][newCol].f || cellDetails[newCol][newRow].f == INT_MAX) // slight change 
			//		{
			//			openList.insert(std::make_pair(newF, std::make_pair(newRow, newCol)));

			//			cellDetails[newRow][newCol].f = newF;
			//			cellDetails[newRow][newCol].g = newG;
			//			cellDetails[newRow][newCol].h = newH;
			//			cellDetails[newRow][newCol].parent = std::make_pair(row, col);
			//		}
			//	}
			//	if (foundDest) break;
			//}
			while (!path.empty())
				path.pop();
			int row, col;
			row = dest.first;
			col = dest.second;

			if (foundDest)
			{
				while (cellDetails[row][col].parent != std::make_pair(row, col))
				{
					path.push(std::make_pair(row, col));
					int newRow = cellDetails[row][col].parent.first;
					int newCol = cellDetails[row][col].parent.second;
					row = newRow;
					col = newCol;
				}

				path.push(std::make_pair(source.first, source.second));

				while (!path.empty() )
				{
					std::cout << " -> " << path.top().first << "," << path.top().second;
					map[path.top().first][path.top().second].setFillColor(sf::Color::Red);
					path.pop();
					std::cout << '\n';
				}
				
			}
			else std::cout << "no dest" << '\n';
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
		window.draw(Inky);
		window.draw(Clyde);
		window.draw(player);

		window.display();
	}
	return 0;
}

