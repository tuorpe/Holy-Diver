/*
*
* Holy diver - an epic adventure at object-oriented world way beneath the surface!
* Template code for implementing the rich features to be specified later on.
*
*/
/****************************************************/
/*                                                  */
/*                  Codign by                       */
/*               Peter Tuoriniemi                   */
/*                    ©2025                         */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <vector>

using namespace std;

/****************************************************/
// Game settings:
/****************************************************/
const string VERSION = "0.02";
const int MAX_BATTERY = 100;
const int MAX_HEALTH = 100;
const int MAX_OXYGEN = 100;
const int INIT_LIVES = 3;
const int OXYGEN_DECREASE_RATE = 2;
const int BATTERY_DECREASE_RATE = 5;

//Points
const int BATTERY_POINTS = 1;
const int OXYGEN_POINTS = 1;
const int COIN_POINTS = 10;
const string g_levels[] = { "level_0.map", "level_1.map", "level_2.map" };

/****************************************************/
// declaring classes:
/****************************************************/
class World;
class PlayerClass;
class GameController;
class Item;
/****************************************************/
// declaring functions:
/****************************************************/
void start_splash_screen(void);
bool startup_routines();
void quit_routines(void);
//World* load_level(string filepath); // a routine to load a level map from a file
int read_input(char*);
//void update_state(char);  // assuming only one input char (key press) at most at a time ("turn-based" execution flow)
//void render_screen(void);
//void delete_map(void);
void gameOver(void);
//void resetGame(void);
//void parse_cells(void);
bool screenLevelFinished(void);



/****************************************************/
// global variables:
/****************************************************/
//char** map;// pointer pointer equals to array of arrays = 2-dimensional array of chars
//size_t map_x, map_y;
GameController* g_gameController = nullptr;

// above is virtually identical, as a variable, compared to for example:
//	    char map[MAXSTR][MAXLEN] = {{0}}; // declare a static 2-dim array of chars, initialize to zero
// However pointer to pointer has not allocated memory yet attached to it, this is done dynamically when actual size known


typedef struct Player {
	int health;
	int oxygen;
	int lives;
	int x;
	int y;
} Player; // named as "Player", a struct containing relevant player data is declared

Player player_data = { MAX_HEALTH, MAX_OXYGEN, INIT_LIVES, 0, 0}; // initialize player data
bool gameRunning;

/****************************************************************
***			Enums											  ***
*****************************************************************/

typedef enum
{
	IDLE,
	UP,
	DOWN,
	LEFT,
	RIGHT,
	RANDOM
}Directions;

typedef enum
{
	oxygen_cell,
	battery_cell,
	empty_cell,
	wall_cell,
	enemy_cell,
	player_cell,
	coin_cell,
}Map_CellTypes;



/****************************************************************
***			Classes											  ***
*****************************************************************/

class Vector2
{
	public:
		int x;
		int y;
		Vector2()
		{
			x = 0;
			y = 0;
		}
		Vector2(int x, int y)
		{
			this->x = x;
			this->y = y;
		}
		Vector2 operator+(const Vector2& other)
		{
			return Vector2(x + other.x, y + other.y);
		}
		Vector2 operator-(const Vector2& other)
		{
			return Vector2(x - other.x, y - other.y);
		}
		Vector2 operator+=(const Vector2& other)
		{
			x += other.x;
			y += other.y;
			return Vector2(x,y);
		}
		Vector2 operator-=(const Vector2& other)
		{
			x -= other.x;
			y -= other.y;
			return Vector2(x,y);
		}

		bool operator==(const Vector2& other)
		{
			if (x == other.x && y == other.y)
				return true;	// Vectors are equal
			else
				return false;	// Vectors are not equal
		}

		bool operator!=(const Vector2& other)
		{
			return !(*this == other);	// Vectors are not equal
		}

		static Vector2 up()
		{
			return Vector2(0, -1);
		}
		static Vector2 down()
		{
			return Vector2(0, 1);
		}
		static Vector2 left()
		{
			return Vector2(-1, 0);
		}
		static Vector2 right()
		{
			return Vector2(1, 0);
		}
		static Vector2 zero()
		{
			return Vector2(0, 0);
		}
		static Vector2 one()
		{
			return Vector2(1, 1);
		}

};

class World
{
private:
	char** p_map;
	char** p_playerMap;	// Map of what player sees
	int p_map_x = 0;	// Map width
	int p_map_y = 0;	// Map height
	void initPlayerMap(bool keepOld = false)
	{
		// Create player's map where he cannot see anything at first
		for (int i = 0; i < p_map_x; i++)
		{
			if (!keepOld)
				p_playerMap[i] = new char[p_map_y + 1];	// Allocate memory for each row
			for (int j = 0; j < p_map_y; j++)
			{
				p_playerMap[i][j] = 0;	// Initialize to darkness
			}
		}

	}
public:
	// Public methods
	World()
	{
		p_map = nullptr;	// Initialize map to null
		p_playerMap = nullptr;	// Initialize player map to null
	}
	World(string filename)
	{
		// Constructor
		load_level(filename);	// Load level from file
		p_playerMap = new char* [p_map_x];	// Allocate memory for player map
		initPlayerMap();
	}
	~World()
	{
		// Destructor
		for (int i = 0; i < p_map_x; i++)
		{
			delete[] p_playerMap[i];	// Free memory for each row
		}
		delete[] p_playerMap;	// Free memory for player map
	}
	void setMap(char** mapSource, Vector2 size)
	{
		p_map = mapSource;	// Set map
		p_map_x = size.x;			// Set map width
		p_map_y = size.y;			// Set map height
		p_playerMap = new char* [p_map_x];	// Allocate memory for player map
		initPlayerMap();	// Initialize player map
	}
	void delete_map(void)
	{
		for (size_t i = 0; i < p_map_y; ++i)
		{
			delete[] p_map[i];
		}
		delete[] p_map;
	}
	Vector2 mapSize()
	{
		return Vector2(p_map_x, p_map_y);	// Return map size
	}
	string printMap()
	{
		string map;
		for (int i = 0; i < p_map_x; i++)
		{
			for (int j = 0; j < p_map_y; j++)
			{
				if (p_map[i][j] == 'P')			// If cell is the player
					map += 'P';					// Print player
				else if (p_map[i][j] == 'x')	// If cell is a wall
					map += 'x';					// Print wall
				/*else if (p_map[i][j] == 'M')  // For debug show enemies
					map += 'M';*/
				else if (p_playerMap[i][j] == 1)// If player can see the cell
					map += p_map[i][j];			// Print cell
				else							// If player can see the cell
					map += ' ';					// Print empty space
			}
			map += '\n';	// New line
		}
		return map;
	}

	void resetPlayerMap()
	{
		initPlayerMap(true);
	}

	bool canMoveTo(Vector2 location)
	{
		// Check if location can be moved into
		if (location.x < 0 || location.x >= p_map_x || location.y < 0 || location.y >= p_map_y)
			return false;	// Out of bounds
		char cellContent = p_map[location.x][location.y];
		if (cellContent != 'x')
			return true;
		return false;
	}
	bool moveOnMap(Vector2 from, Vector2 to, Map_CellTypes item)
	{
		if (canMoveTo(to))	// Check if item can be moved to new position
		{
			putOnMap(from, empty_cell);	// Remove item from old position
			putOnMap(to, item);			// Put item on map
			if (item == player_cell)	//Mark new cell as visible to player.
			{
				showCell(from);
				showCell(to);
			}
			return true;				// Item moved
		}
		return false;	// Item not moved
	}
	char getCell(Vector2 position)
	{
		return p_map[position.y][position.x];
	}

	void putOnMap(Vector2 position, Map_CellTypes item)
	{
		char p_item = 'o';
		// Put thing on map
		switch (item)
		{
		case empty_cell:
			p_item = 'o';
			break;
		case wall_cell:
			p_item = 'x';
			break;
		case enemy_cell:
			p_item = 'M';
			break;
		case player_cell:
			p_item = 'P';
			break;
		}
		p_map[position.y][position.x] = p_item;
	}
	void showCell(Vector2 position, Vector2 direction = Vector2::zero())
	{
		Vector2 lightedCell = position + direction;
		p_playerMap[lightedCell.y][lightedCell.x] = 1;	// Light the cell
	}
	/****************************************************************
	 *
	 * FUNCTION load_level
	 *
	 * Open a map file and load level map from it.
	 * First weekly home assignment is to be implemented mostly here.
	 *
	 * **************************************************************/
	void load_level(string filepath)
	{
		// steps in short:
		// 1) locate, check and open file, if failure, return value indicating error (and check on the calling side)
		// 2) read first row, count number of characters. Assuming all maps are rectangular, use this information
		//    to memory allocation of global "map" (char ** map) pointer.
		//    Assuming first row contains N characters, then you need to allocate 2D table/array of dimensions N x N
		//    -> in practice first allocate to "map" an N-long array of (char *) pointers
		//        and then within a loop allocate an N-long array of chars to each of the previous entries
		//           -> as a result "map" is a pointer to pointer corresponding a 2D-array sized [N][N]
		//              and it's each "slot" can be referred to using syntax: map[x][y], each capable of storing a char.
		// 3) close file
		// 4) return with success value (e.g. zero when OK, negative if error)
		// [  5) outside this function, remember to free() allocated memory eventually ]
		ifstream loaded_map(filepath);
		//size_t rows, map_x;
		//If file not found
		if (!loaded_map)
		{
			cout << "Map file not found" << endl;
			p_map = nullptr;
		}
		if (loaded_map.is_open())
		{
			string line;
			getline(loaded_map, line);	// Read first line
			p_map_x = line.length();		// Get the size of the line
			if (p_map_x == 0)
				p_map = nullptr;
			p_map_y = 1;
			p_map = new char* [p_map_x];		//Assuming map is square
			for (int i = 0; i < p_map_x; i++)	// Allocates map array and fills with data from the file
			{
				p_map[i] = new char[p_map_x + 1];
				for (int j = 0; j < p_map_x; j++)
				{
					p_map[i][j] = line[j];
				}
				p_map[i][p_map_x] = '\0';
				if (loaded_map.eof())
					break;
				//Read new line
				getline(loaded_map, line);
				p_map_y++;
			}
			/*cout << "Loaded map:" << endl;
			for (int i = 0; i < cols; i++)
			{
				cout << map[i] << endl;
			}
			cout << "Player is at: x: " << player_data.x << " y: " << player_data.y << endl;
			*/

		}
	}

};

class Character
{
	protected:
		Vector2 p_position;
		Map_CellTypes p_cellType;
		World* p_world;
		Directions getRandomDirection()
		{
			// Get random direction
			int random = rand() % 5;	// Get random number between 0 and 3
			return static_cast<Directions>(random);	// Cast to Directions enum 
		}

	public:
		Character()
		{
			p_position = Vector2(0 ,0);
			p_cellType = empty_cell;	// Set cell type to empty
			p_world = nullptr;	// Initialize world to null
		}

		Character(Vector2 position, World* world, Map_CellTypes cellType)
		{
			this->p_position = position;
			p_world = world;	// Set world
			p_cellType = cellType;	// Set cell type
		}

		Vector2 GetPosition()
		{ 
			return p_position;
		}

		void setPosition(Vector2 newLocation)
		{
			if (p_world->canMoveTo(newLocation))	// Check if new location is valid
				p_position = newLocation;
		}

		virtual void move(Vector2 movement = Vector2::one())
		{
			if (movement == Vector2::one())
				movement = getMovementVector(getRandomDirection());	// Get random direction
			Vector2 newPos = p_position + movement;
			//Check if out of bounds
			if (p_world->canMoveTo(newPos) == false)	// Check if new position is valid
				return;	// Out of bounds
			if (p_world->moveOnMap(p_position, newPos, p_cellType))
				p_position = newPos;	// Move enemy
		}

		static Vector2 getMovementVector(Directions direction)
		{
			switch (direction)
			{
			case UP:
				return Vector2::up();		// Get up vector
				break;
			case DOWN:
				return Vector2::down();	// Get down vector
				break;
			case LEFT:
				return Vector2::left();	// Get left vector
				break;
			case RIGHT:
				return Vector2::right();	// Get right vector
				break;
			default:
				return Vector2(0, 0);	// Get idle vector
				break;
			}
		}
};

class PlayerClass:public Character
{
	private:
		int p_batterySOC;
		int p_hp;
		int p_oxygen;
		int p_lives;
		int p_score;
public:
		PlayerClass() :Character()
		{
			//Constructor
			p_batterySOC = 0;
			p_hp = 0;
			p_oxygen = 0;
			p_lives = 0;
			p_world = nullptr;	// Initialize world to null

		}
		PlayerClass(Player player_data, World* world, int maxBattery)
		{
			//Constructor
			this->p_position.x = player_data.x;
			this->p_position.y = player_data.y;
			p_hp = player_data.health;
			p_oxygen = player_data.oxygen;
			p_lives = player_data.lives;
			p_batterySOC = maxBattery;
			p_world = world;	// Set world
			p_cellType = player_cell;
		}

		~PlayerClass()
		{
			//Destructor
			cout << "RIP Player" << endl;
			gameOver();
		}

		void SetWorld(World* world)
		{
			p_world = world;
		}
		bool useBattery()
		{
			// Use battery
			if (p_batterySOC > BATTERY_DECREASE_RATE)
			{
				p_batterySOC -= BATTERY_DECREASE_RATE;
				return true;	// Battery used
			}
			else
			{
				p_batterySOC = 0;	// No battery left
				return false;		
			}
		}

		int getBattery()
		{
			return p_batterySOC;	// Get battery
		}

		void resetBattery()
		{
			p_batterySOC = MAX_BATTERY;	// Reset battery
		}

		bool receiveDamage(int damage)
		{
			if (p_hp > damage)
			{
				p_hp -= damage;
				return true;	// Character is still alive
			}
			else
			{
				p_hp = 0;
				return false;	// Character is dead
			}
		}

		bool useOxygen()
		{
			if (p_oxygen > 0)
			{
				p_oxygen--;
				return true;
			}
			if (receiveDamage(OXYGEN_DECREASE_RATE))
				return true;
			return false;
		}

		int getOxygen()
		{
			return p_oxygen;
		}

		void setOxygen(int oxygen)
		{
			if(oxygen > 0)	
				p_oxygen = oxygen;
		}

		int getHealth()
		{
			return p_hp;
		}

		void setHealth(int health)
		{
			if(health > 0)
				p_hp = health;
		}

		void useLife()
		{
			if (p_lives)
				p_lives--;
		}

		void setLives(int lives)
		{
			if (lives > 0)
				p_lives = lives;
		}

		int getLives()
		{
			return p_lives;
		}

		int GetScore()
		{
			return p_score;	// Get score
		}

		void incrementScore(int value)
		{
			if (value < 0)
				return;
			p_score += value;
		}

		void resetScore()
		{
			p_score = 0;
		}

		bool isAlive()
		{
			if (p_hp > 0 && p_lives)
				return true;	// Character is alive
			else
				return false;	// Character is dead
		}
		void resetPLayer()
		{
			setHealth(MAX_HEALTH);	// Reset player health
			setOxygen(MAX_OXYGEN);	// Reset player oxygen
			setLives(INIT_LIVES);	// Reset player lives
			setPosition(Vector2(player_data.x, player_data.y));	// Reset player position
			resetScore();
		}

};

class Enemy:public Character
{
	private:
		bool isActive;
	public:
		Enemy(Vector2 location, World* world)
		{
			//Constructor
			isActive = false; // Payer has not seen this enemy yet
			p_world = world;
			p_position = location;
			p_cellType = enemy_cell;
		};
		~Enemy()
		{
			//Destructor
			//cout << "Enemy killed" << endl;

		}
		void setActive()
		{
			isActive = true;	// Set active
		}

		void getDamage(int amount)
		{
			// Player hurts enemy
		};
		int doDamage()
		{
			// Hurt player
			return 1;	// Return damage amount
		};
};

class IdleEnemy : public Enemy
{
public:
	IdleEnemy(Vector2 location, World* world) : Enemy(location, world)
	{
		//Constructor
	};
	~IdleEnemy()
	{
		//Destructor
	}
	void move()
	{
		// Do not move
	}
};

class Item
{
	private:
		int points;
		Map_CellTypes type;
		Vector2 position;
	public:
		Item(Vector2 location, Map_CellTypes type, int points): type(type), position(location)
		{
			//Constructor
			if (points >= 0)
				this->points = points;
			else
				this->points = 0;
		}
		~Item()
		{
			//Destructor
		}
		int GetPoints()
		{
			return points;
		}
		Map_CellTypes GetType()
		{
			return type;
		}

		Vector2 GetPosition()
		{
			return position;
		}
		/*void putOnMap(World& map)
		{
			// Place item on map
		}*/
};

class GameController
{
private:
	World* p_world;
	PlayerClass* p_player;
	vector<Enemy*> p_enemies;	// List of enemies
	vector<Item*> p_items;		// List of items
	Map_CellTypes p_foundItem;
	int p_currentLevelIndex;
	int p_startCoinAmount;
	template<typename T> 
	bool isAtPlayerPosition(const T& item)
	{
		// Check if position is same as player position
		return p_player->GetPosition() == item->GetPosition();
	}
public:
	GameController()
	{
		// Constructor
		p_world = nullptr;
		p_player = nullptr;
		p_foundItem = empty_cell;	// Initialize found item
		p_startCoinAmount = 0;	// Initialize start coin amount
		p_currentLevelIndex = 0;	// Initialize current level index
	}

	GameController(World* world)
	{
		// Constructor
		p_world = world;
		p_player = nullptr;
		p_foundItem = empty_cell;	// Initialize found item
		p_startCoinAmount = 0;	// Initialize start coin amount
		p_currentLevelIndex = 0;	// Initialize current level index
	}

	~GameController()
	{
		// Destructor
		ClearMemory();
		//cout << "GameController destroyed" << endl;
	}

	void SetWorld(World* world)
	{
		p_world = world;
	}

	void ClearMemory()
	{
		for (Enemy* enemy : p_enemies)
		{
			delete enemy;	// Free memory for each enemy
		}
		p_enemies.clear();
		for (Item* item : p_items)
		{
			delete item;	// Free memory for each item
		}
		p_items.clear();
		delete p_player;	// Free memory for player
		delete p_world;

	}
	void addEnemy(Enemy* enemy)
	{
		p_enemies.push_back(enemy);	// Add enemy to the list
		//p_world->putOnMap(enemy->getPosition(), enemy_cell);	// Put enemy on map
	}

	int getAmount(Map_CellTypes type)
	{
		int amount = 0;
		for (Item* item : p_items)
		{
			if (item->GetType() == type)
				amount++;
		}
		return amount;
	}

	void addPlayer(Player player)
	{
		// Add player to the world
		p_player = new PlayerClass(player_data, p_world, MAX_BATTERY);	// Create player
		Vector2 player_location = p_player->GetPosition();	// Get player's location
		p_world->putOnMap(player_location, player_cell);	// Put player on map
		p_world->showCell(player_location);
	}

	void addItem(Item* item)
	{
		p_items.push_back(item);
		if (item->GetType() == coin_cell)
			p_startCoinAmount++;	// Increase start coin amount
	}

	void updatePlayer(Vector2 movement)
	{
		if (!p_player->useOxygen())	// Try to use oxygen
			return;
		p_player->move(movement);
	}

	void updateEnemies()
	{
		for (Enemy* enemy : p_enemies)
		{
			enemy->move(Vector2::one()); // Move enemy by vector one. Indicating random movement
		}
	}
	
	Map_CellTypes getFoundItem()
	{
		Map_CellTypes item = p_foundItem;	// Get found item
		p_foundItem = empty_cell;	// Reset found item
		return item;	// Return found item
	}
	
    void CheckPlayerHit()
    {
        // Check if player hit an enemy
		auto hitEnemy = find_if(p_enemies.begin(), p_enemies.end(), [this](Enemy* enemy) 
		{
			return isAtPlayerPosition(enemy);
		});
        if(hitEnemy != p_enemies.end())
        {
			if ((*hitEnemy)->GetPosition() == p_player->GetPosition()) // Check if player hit an enemy
            {
				(*hitEnemy)->setActive(); // Set enemy active
                p_player->receiveDamage((*hitEnemy)->doDamage()); // Hurt player
            }
        }
        // Check items if any of them match the playe's position
        auto foundItem = find_if(p_items.begin(), p_items.end(), [this](Item* item) 
		{
            return isAtPlayerPosition(item);
        });

        if (foundItem != p_items.end()) // Check if player hit an item
        {
            if ((*foundItem)->GetType() == battery_cell) // Check if item is a battery
                p_player->resetBattery();
            else if ((*foundItem)->GetType() == oxygen_cell) // Check if item is oxygen
                p_player->setOxygen(MAX_OXYGEN);
			p_player->incrementScore((*foundItem)->GetPoints());
            p_foundItem = (*foundItem)->GetType();  // Set found item to be shown on screen
            delete (*foundItem);					// Free memory for item
            p_items.erase(foundItem);				// Remove item from list
        }
    }
		
	void useFlashLight(Directions direction)
	{
		if (!p_player->useBattery())	// Try to use flashlight battery
			return;
		Vector2 lightCellPos = p_player->GetPosition() + Character::getMovementVector(direction);
		p_world->showCell(lightCellPos);	// Show cell to player
		for (Enemy* enemy : p_enemies)		//Activate enemy if player sees it
		{
			if (enemy->GetPosition() == lightCellPos)	// Only 1 enemy can be in one place so if found -> return
			{
				enemy->setActive();
				return;
			}
		}
	}

	int GetScore()
	{
		return p_player->GetScore();
	}
	
	void CheckPlayerAlive()
	{
		if (p_player->isAlive())
			return;
		while (true)
		{
			system("cls");
			cout << "\tGAME OVER" << endl << endl;
			cout << "Your final score: " << p_player->GetScore() << endl << endl;
			cout << "Press Enter key to restart game" << endl;
			cout << "Press Q to quit game" << endl;
			char input;
			if (read_input(&input) < 0)
			{
				gameRunning = false;
				return;
			}
			else if (input == '\n')
			{
				ResetGame();
				return;
			}
		}
	}

	bool CheckLevelFinished()
	{
		if (getAmount(coin_cell) <= 0)
			return true;
		return false;
	}

	void loadNextLevel()
	{
		string levelName;
		delete p_world;				// Delete old world
		p_enemies.clear();			// Forget old enemies
		p_items.clear();			// Forget old items
		p_currentLevelIndex++;
		if (p_currentLevelIndex >= sizeof(g_levels) / sizeof(g_levels[0]))
		{
			p_currentLevelIndex = 0;
		}
		levelName = g_levels[p_currentLevelIndex];
		p_world = new World(levelName);	// Create new world
		p_player->SetWorld(p_world);	// Set new world information for player
		parse_cells();
		render_screen();
	}
	
	void ResetGame()
	{
		ClearMemory();
		startup_routines();
	}
	/****************************************************************
	 *
	 * FUNCTION parse_enemies()
	 *
	 * function finds enemies on maps and adds them to the world
	 *
	 * **************************************************************/
	void parse_cells()
	{
		IdleEnemy* p_idleEnemy;
		Enemy* p_enemy;
		Item* p_item;
		p_startCoinAmount = 0;
		Vector2 map_size = p_world->mapSize();
		// Loop through the map and find all items and enemies
		for (int i = 0; i < map_size.y; i++)
		{
			for (int j = 0; j < map_size.x; j++)
			{
				Vector2 pos = Vector2(j, i);
				switch (p_world->getCell(pos))
				{
				case 'm':	// Immobile enemy
					p_idleEnemy = new IdleEnemy(pos, p_world);
					addEnemy(p_idleEnemy);
					break;
				case 'M':	// Mobile enemy
					p_enemy = new Enemy(pos, p_world);
					addEnemy(p_enemy);
					break;
				case 'b':	// Battery
					p_item = new Item(pos, battery_cell, BATTERY_POINTS);
					addItem(p_item);
					break;
				case 'O':	// Oxygen
					p_item = new Item(pos, oxygen_cell, OXYGEN_POINTS);
					addItem(p_item);
					break;
				case 'c':	// Coin
					p_item = new Item(pos, coin_cell, COIN_POINTS);
					addItem(p_item);
					break;
				case 'P':
					p_player->setPosition(pos);
					break;
				default:

					break;
				}
			}
		}
	}

	void exit()
	{
		while (true)
		{
			system("cls");
			cout << "\tExit" << endl;
			int coins = getAmount(coin_cell);
			cout << "You found " << coins << " coins out of " << p_startCoinAmount << endl;
			cout << "Press:" << endl;
			bool canContinue = (float)coins / (float)p_startCoinAmount < 0.5;
			if (canContinue)
			{
				cout << "D: Move to next level but you will loose all your items" << endl;
			}
			cout << "C: Continue playing" << endl;
			cout << "R: Retry level" << endl;
			cout << "Q: Quit game" << endl;
			char input;
			if (read_input(&input) < 0)
			{
				gameRunning = false;
				return;
			}
			switch (input)
			{
				case 'd':
					if (canContinue)
					{
						p_player->resetPLayer();	// Reset player
						loadNextLevel();
						return;
					}
					break;
				case 'c':
					return;
					break;
				case 'r':
					p_player->resetPLayer();	// Reset player
					
					delete p_world;				// Delete current world
					p_world = new World(g_levels[p_currentLevelIndex]);	// Create same world again
					parse_cells();
					break;
				default:
					continue;
					break;
			}
			return;
		}
	}

	/****************************************************************
	 *
	 * METHOD update_state
	 *
	 * update game state (player, enemies, artefacts, inventories, health, whatnot)
	 * this is a collective entry point to all updates - feel free to divide these many tasks into separate subroutines
	 *
	 * **************************************************************/
	void update_state(char input)
	{
		switch (input)
		{
		case 'w':	// Go up
			updatePlayer(Vector2::up());
			break;
		case 'a':	// Go left
			updatePlayer(Vector2::left());
			break;
		case 's':	// Go down
			updatePlayer(Vector2::down());
			break;
		case 'd':	// Go right
			updatePlayer(Vector2::right());
			break;
		case 'i':
			useFlashLight(UP);
			break;
		case 'k':
			useFlashLight(DOWN);
			break;
		case 'j':
			useFlashLight(LEFT);
			break;
		case 'l':
			useFlashLight(RIGHT);
			break;
		case 'r':	// Reset level
			exit();
			break;
		default:
			break;
		}
	}

	void run()
	{
		updateEnemies();	// Update enemies position/Tick
		CheckPlayerHit();	// Check if player hit an enemy
		CheckPlayerAlive();

	}

	/*****************************************************************
 *
 * FUNCTION render_screen
 *
 * this function prints out all visuals of the game (typically after each time game state updated)
 *
 * **************************************************************/
	void render_screen(void)
	{
		if (!gameRunning)
			return;
		system("cls");
		cout << "PLAYER:" << endl << "Health:\t" << p_player->getHealth() << '%' << "\tOxygen : " << p_player->getOxygen() << endl;
		cout << "Battery: " << p_player->getBattery() << '%' << endl;
		cout << "Coins: " << getAmount(coin_cell) << " / " << p_startCoinAmount << "\tScore: " << p_player->GetScore() << " pts" << endl;
		Vector2 location = p_player->GetPosition();
		cout << "Player coordinates: x: " << location.x << " y: " << location.y << endl;
		cout << p_world->printMap() << endl;
		Map_CellTypes foundItem = getFoundItem();
		string type = "";
		switch (foundItem)
		{
		case coin_cell:
			type = "Coin";
			break;
		case battery_cell:
			type = "Battery";
			break;
		case oxygen_cell:
			type = "Battery";
			break;
		}
		if (type != "")
			cout << "Found an item: " << type << endl;
	}

};

/****************************************************************
 *
 * MAIN
 * main function contains merely function calls to various routines and the main game loop
 *
 ****************************************************/
int main(void)
{	
#ifdef _WIN32 || _WIN64
	SetConsoleOutputCP(CP_UTF8);
#endif

	while (true)
	{
		start_splash_screen();
		if (!startup_routines())
		{
			cout << "Startup failed" << endl;
			continue;
		}
		char input;
		Sleep(2000);
		system("cls");
		g_gameController->render_screen();
		// IMPORTANT NOTE: do not exit program without cleanup: freeing allocated dynamic memory etc
		while (true) // infinite loop, should end with "break" in case of game over or user quitting etc.
		{
			if (!gameRunning)
			{
				cout << "Game over" << endl;
				break;
			}
			input = '\0'; // make sure input resetted each cycle
			//if (0 > read_input(&input)) break; // exit loop in case input reader returns negative (e.g. user selected "quit")
			if (read_input(&input) == -2)
			{
				g_gameController->exit();
			}
			g_gameController->update_state(input);
			g_gameController->run();	// Update game state
			g_gameController->render_screen();
			if (g_gameController->CheckLevelFinished())
			{
				if (!screenLevelFinished()) // Show that level is finished and ask if player wants to continue
				{
					break; // Quit normal play loop
				}
				//Load next level
				else
				{
					g_gameController->loadNextLevel();	// Load next level
				}

			}
		}

		quit_routines(); // cleanup, bye-bye messages, game save and whatnot
		system("pause");
		return 0;
	}
}


/****************************************************************
 *
 * FUNCTION read_input
 *
 * read input from user
 *
 * **************************************************************/
int read_input(char* input)
{
	cout << ">>>";	// simple prompt
	try {
		*input = getchar();
	}
	catch (...) {
		return -1; // failure
	}
	cout << endl;  //new line to reset output/input
	cin.ignore();  //clear cin to avoid messing up following inputs
	if (*input == 'q') return -2; // quitting game...
	return 0; // default return, no errors
}

/*****************************************************************
**	FUNCTION gameOver											**
**	This will signal running game to stop						**
******************************************************************/
void gameOver()
{
	gameRunning = false;
}

/******************************************************************
** FUNCTION screenLevelFinished									 **
** This will show that level is finished and ask if player		 **
** wants to continue. If player press enter, it will return true **
** If player press q, it will return false.						 **
*******************************************************************/
bool screenLevelFinished()
{
	char input;
	system("cls");
	cout << "\tLEVEL FINISHED!" << endl;
	cout << "\tScore: " << g_gameController->GetScore() << endl;
	while (true)
	{
		cout << "\tPress enter to continue" << endl;
		cout << "\tPress q to quit" << endl;
		read_input(&input);
		if (input > 0)
		{
			if (input == '\n')
				return true;
		}
		return false;
	}
}



/****************************************************************
 *
 * FUNCTION start_splash_screen
 *
 * outputs any data or info at program start
 *
 * **************************************************************/
void start_splash_screen(void)
{
	/* this function to display any title information at startup, may include instructions or fancy ASCII-graphics */
	cout << endl << "WELCOME to epic Holy Diver v." << VERSION << endl;
	cout << "Enter commands and enjoy!" << endl << endl;
	cout << "After each command press Enter key" << endl;
	cout << "Guide:" << endl;
	cout << "Movement:" << endl << endl;
	cout << "\tW" << u8"\u2191" << endl;
	cout << u8"\u2190" << "A\t\tD" << u8"\u2192" << endl;
	cout << "\tS" << u8"\u2193" << endl << endl;
	cout << "Flashlight:" << endl << endl;
	cout << "\tI" << u8"\u2191" << endl;
	cout << u8"\u2190" << "J\t\tL" << u8"\u2192" << endl;
	cout << "\tK" << u8"\u2193" << endl << endl;

	cout << "Commands:" << endl;
	cout << "Q: Quit game" << endl;
	cout << "R: Retry game" << endl;
	//cin.ignore();
}

/****************************************************************
 *
 * FUNCTION startup_routines
 *
 * Function performs any tasks necessary to set up a game
 * May contain game load dialogue, new player/game dialogue, level loading, random inits, whatever else
 *
 * At first week, this could be suitable place to load level map.
 *
 * **************************************************************/
bool startup_routines()
{
	cout << "Name of the level file (default go to level 1): ";
	string levelName;
	getline(cin, levelName);
	if (levelName.empty())
	{
		levelName = g_levels[0];
	}
	World* world = new World(levelName);

	// Add GameController if not yet made
	if (g_gameController == nullptr)
		g_gameController = new GameController(world);
	else
		g_gameController->SetWorld(world);
	// Add player
	g_gameController->addPlayer(player_data);
	//Add enemies and items to the world
	g_gameController->parse_cells();
	gameRunning = true;
	return true;
	// For example if memory allocated here... (*)
}


/****************************************************************
 *
 * FUNCTION quit_routines
 *
 * function performs any routines necessary at program shut-down, such as freeing memory or storing data files
 *
 * **************************************************************/
void quit_routines(void)
{

	// (*) ... the memory should be free'ed here at latest
	delete g_gameController;
	cout << endl << "BYE! Welcome back soon." << endl;
}

