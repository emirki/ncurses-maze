//	Introduction to Programming (C++) Lab Project

#include <stdio.h>
#include <cstdlib>
#include <ctime>
#include <string>

#if defined(_WIN32) | defined(_WIN64)
#include <conio.h>
#include <Windows.h>
#elif defined(__linux__)
#include <ncurses.h>
#else
#error "unknown os"
#endif

struct vec2
{
	int x;
	int y;
};

enum BonusType : unsigned int
{
	NONE = 0,
	NOCLIP = 1,
	EXTRA_TIME = 2,
	EXTRA_SCORE = 3,
	LOSE_TIME = 4,
	LOSE_SCORE = 5,
	WINLEVEL = 6
};

enum NodeType
{
	NOTHING,
	WALL,
	BONUS,
	CAR,
	GOAL
};

struct Node
{
public:
	void SetPos(int x,int y)
	{
		pos.x = x;
		pos.y = y;
	}
	NodeType type;
	vec2 pos;
};

struct Car : public Node
{
	Car()
	{
		SetPos(0, 0);
	}

	bool Move(const vec2& movement);
	void Draw();
	BonusType activeBonus;
};

struct Bonus : public Node
{
	int id;
	bool visible;
	BonusType bonus;
};

const vec2 UP = { 0, -1 };
const vec2 DOWN = { 0, 1 };
const vec2 RIGHT = { 1, 0 };
const vec2 LEFT = { -1, 0 };

const unsigned int BONUS_COUNT = 4;
const unsigned int SCORE_COUNT = 5;

const vec2 LOOK_AROUND[4] = {
	RIGHT,
	UP,
	LEFT,
	DOWN
};

void DrawGameArea();
void DrawBonuses();
void DrawScore();
void DrawHighScores();

#define GAME_AREA_WIDTH 2*30+1
#define GAME_AREA_HEIGHT 2*10+1

#define OFFSET_X 5
#define OFFSET_Y 5

Node game_area[GAME_AREA_WIDTH][GAME_AREA_HEIGHT];
Node goal;
Bonus bonus_nodes[BONUS_COUNT];
Car *car;

int score;
unsigned int scores[SCORE_COUNT];

bool visited[GAME_AREA_WIDTH][GAME_AREA_HEIGHT];

void InitScreen()
{
	initscr();
	noecho();
	cbreak();
	nodelay(stdscr, TRUE);
	keypad(stdscr, TRUE);
	refresh();
}

/**
 *
 */
void DisposeScreen()
{
	clrtoeol();
	refresh();
	endwin();
}

/**
 *	Labirent yaratimi icin Depth-First Algoritmasi
 *	TODO: Bu kod calinti. Bu kodun nasil isledigini anlamak lazim...
 */
void DFS(int x, int y)
{
	if (visited[x][y])
		return;
	visited[x][y] = TRUE;
	game_area[x][y].type = NOTHING;
	char unvisited_neighbour_counter;
	do
	{
		unvisited_neighbour_counter = 0;
		int i;
		for (i = 0; i < 4; i++)
		{
			int _x = x + (2 * LOOK_AROUND[i].x);
			int _y = y + (2 * LOOK_AROUND[i].y);
			if (GAME_AREA_HEIGHT > _y && _y > 0 && GAME_AREA_WIDTH > _x
					&& _x > 0 && visited[_x][_y] == FALSE)
			{
				unvisited_neighbour_counter++;
			}
		}
		if (unvisited_neighbour_counter > 0)
		{
			int internal_counter = -1;
			int j = -1;
			char choice = rand () % (unvisited_neighbour_counter + 1);
			do
			{
				int _x = x + (2 * LOOK_AROUND[j].x);
				int _y = y + (2 * LOOK_AROUND[j].y);
				if (GAME_AREA_HEIGHT > _y && _y > 0 && GAME_AREA_WIDTH > _x
						&& _x > 0 && visited[_x][_y] == FALSE)
					internal_counter++;
				j++;

			}
			while (internal_counter != choice - 1);
			j--;
			game_area[x + LOOK_AROUND[j].x][y + LOOK_AROUND[j].y].type = NOTHING;
			DFS(x + (2 * LOOK_AROUND[j].x), y + (2 * LOOK_AROUND[j].y));
		}
	}
	while (unvisited_neighbour_counter > 0);
}

/**
 *
 */
void InitGameArea()
{
	for (unsigned int i = 0; i < GAME_AREA_WIDTH; i++)
	{
		for (unsigned int j = 0; j < GAME_AREA_HEIGHT; j++)
		{
			game_area[i][j].type = WALL;
			visited[i][j] = FALSE;
		}
	}

	goal = Node();
	goal.type = NodeType::GOAL;
	goal.pos.x = 1;
	goal.pos.y = 1;
	DFS(1, 1);
}

/**
 *
 */
void InitHighScores()
{
	for (unsigned int i = SCORE_COUNT; i = 0; i--)
	{
		scores[i - SCORE_COUNT] = (1000 * i);
	}
}

/**
 *
 */
void InitBonuses()
{
	unsigned int x, y;
	for (unsigned int i = 0; i < BONUS_COUNT; ++i)
	{
		do {
			x = rand() % (GAME_AREA_WIDTH - 1) + 1;
			y = rand() % (GAME_AREA_HEIGHT - 1) + 1;
		}
		while (game_area[x][y].type == WALL);
		bonus_nodes[i].pos.x = x;
		bonus_nodes[i].pos.y = y;
		bonus_nodes[i].type = BONUS;
		bonus_nodes[i].bonus = static_cast<BonusType>(rand() % 5 + 1);
		bonus_nodes[i].id = i;
		bonus_nodes[i].visible = true;
	}
}

/**
 *
 */
void DrawGameArea()
{
	for (unsigned int i = 0; i < GAME_AREA_WIDTH; i++)
	{
		for (unsigned j = 0; j < GAME_AREA_HEIGHT; j++)
		{
			move (j + OFFSET_X, i + OFFSET_Y);
			if (game_area[i][j].type == WALL)
				if (car->activeBonus == BonusType::NOCLIP)
					addch('#');
				else
					addch(' ' | A_REVERSE);
			else
				addch(' ');
		}
	}
	move(OFFSET_X + 1, OFFSET_Y + 1);
	addch('X');
}

/**
 *	TODO: Bu struct icine tasinacak
 */
void Car::Draw()
{
	move(car->pos.y + OFFSET_Y, car->pos.x + OFFSET_X);
	addch(ACS_DIAMOND);
}

/**
 *
 */
void DrawBonuses()
{
	for (unsigned int i = 0; i < BONUS_COUNT; ++i)
	{
		if (bonus_nodes[i].visible == true)
		{
			move (bonus_nodes[i].pos.y + OFFSET_Y, bonus_nodes[i].pos.x + OFFSET_X);
			addch (ACS_BOARD | A_BLINK);
		}
	}
}

/**
 *
 */
bool Car::Move(const vec2& movement)
{
	//mvprintw(26, 0, "x:\t%i, y:\t%i", car->pos.x, car->pos.y);

	if (((car->pos.x > 0 && car->pos.x < GAME_AREA_WIDTH) &&
		(car->pos.y > 0 && car->pos.y < GAME_AREA_HEIGHT)) &&
		((game_area[car->pos.x + movement.x][car->pos.y + movement.y].type == NOTHING) ||
		(game_area[car->pos.x + movement.x][car->pos.y + movement.y].type == BONUS) ||
		(car->activeBonus == NOCLIP)))
	//if (car->activeBonus == BonusType::NOCLIP || ((game_area[car->pos.x + movement.x][car->pos.y + movement.y].type == NOTHING) || (game_area[car->pos.x + movement.x][car->pos.y + movement.y].type == BONUS)))
	{
		mvprintw (car->pos.y + OFFSET_Y, car->pos.x + OFFSET_X, " ");

		car->pos.x += movement.x;
		car->pos.y += movement.y;

		for (unsigned int i = 0; i < BONUS_COUNT; i++)
		{
			if (bonus_nodes[i].pos.x == car->pos.x && bonus_nodes[i].pos.y == car->pos.y && bonus_nodes[i].visible == true)
			{
				bonus_nodes[i].visible = false;
				mvprintw(30, 0, "Bonus %i is hit", bonus_nodes[i].id);
				mvprintw(31, 0, "Bonus %i is %i", bonus_nodes[i].id, bonus_nodes[i].bonus);
				score += 1000;
			}
			else
			{
				//mvprintw(25, 0, "");
			}
		}

		return true;
	}
	else
	{
		move (0, 0);
		return false;
	}
}

/**
 *
 */
void DrawTime(double timer)
{
	mvprintw(2,0, "Time left:\t%d", timer);
}

/**
 *
 */
void DrawScore()
{
	mvprintw(3, 0, "Score:\t%i", score);
}

/**
 *
 */
void DrawHighScores()
{
	mvprintw(1,0," --- HIGH SCORES --- ");

	for (unsigned int i = 0; i < SCORE_COUNT; i++)
	{
		mvprintw(3+i, 0, "%i:\t%i", i+1, scores[i]);
	}

	mvprintw(10,0,"Press Escape key to Exit.");
}

/**
 *	main
 */
int main()
{
	bool done = false;
	unsigned int winCount = 0;
	unsigned int ch;
	std::clock_t t;
	unsigned int duration = 0;
	char cheatString[10];

	srand(time(NULL));

	t = std::clock();

	InitScreen();
	curs_set(0);

	car = new Car();
	car->SetPos(GAME_AREA_WIDTH - 2, GAME_AREA_HEIGHT - 2);
	car->type = CAR;
	car->activeBonus = BonusType::NONE;

	InitGameArea();
	InitBonuses();
	InitHighScores();

	while (!done)
	{
		mvprintw(0, 0, "Intro. to Programming (C++) Lab Project");
		mvprintw(1, 0, "Level:\t%i", winCount + 1);

		DrawGameArea();
		car->Draw();
		DrawBonuses();
		DrawTime(duration);
		DrawScore();

		refresh();

		// TODO: Ayri void Input(char) fonksiyonuna tasinabilir.
		ch = wgetch(stdscr);
		switch (ch)
		{
			case 'w':
			case 'W':
			case KEY_UP:
				if (car->Move(UP))
					score++;
				break;
			case 's':
			case 'S':
			case KEY_DOWN:
				if (car->Move(DOWN))
					score++;
				break;
			case 'd':
			case 'D':
			case KEY_RIGHT:
				if (car->Move(RIGHT))
					score++;
				break;
			case 'a':
			case 'A':
			case KEY_LEFT:
				if (car->Move(LEFT))
					score++;
				break;
			case 'C':
				if (car->activeBonus == BonusType::NONE)
					car->activeBonus = BonusType::NOCLIP;
				else
					car->activeBonus = BonusType::NONE;
				break;
			case 'q':
			case 'Q':
			case 27:
				done = true;
				break;
			default:
				break;
		}

		duration += ((clock() - t) / CLOCKS_PER_SEC);

		if ((goal.pos.x == car->pos.x) && (goal.pos.y == car->pos.y))
		{
			winCount++;
			// TODO: oyunu resetleme icin bi fonksiyon gerekli mi?
			InitGameArea();
			InitBonuses();
			car->SetPos(GAME_AREA_WIDTH - 2, GAME_AREA_HEIGHT - 2);
		}

		if (winCount == 2)
		{
			clear();

			DrawHighScores();

			while (ch = wgetch(stdscr))
			{
				if (ch == 27)
				{
					done = true;
					break;
				}
			}
		}
	}

	delete car;

	DisposeScreen();

	return 0;
}
