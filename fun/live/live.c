#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

#define MAIN_FIELD_WIDTH 100 /* ширина */
#define MAIN_FIELD_HEIGHT 40 /* высота */
#define GENOM_SIZE 4

#define TTL_DEF 50
#define TTL_IS_GOOD 1
#define TTL_IS_NORMAL 2
#define TTL_IS_BAD 3

typedef enum
{
	GEN_NONE,

	GEN_UP,
	GEN_DOWN,
	GEN_LEFT,
	GEN_RIGHT,

	GEN_REPROD,
	GEN_DIE,

	GEN_MAX
} gen_t;

typedef struct
{
	gen_t genom[GENOM_SIZE];
	int gen;
	int step_is_done;
	int ttl;
	int view;
} cell_t;

char *gen_strings[GEN_MAX+1] = 
{
	"NONE",
	"UP",
	"DOWN",
	"LEFT",
	"RIGHT",

	"REPROD",
	"DEI",

	"UNKN"
};

static cell_t main_field[MAIN_FIELD_HEIGHT][MAIN_FIELD_WIDTH] = {{0}, {0}};
static int population = 0;

static char *genom_str(gen_t gen)
{
	switch(gen)
	{
		case GEN_NONE:
		case GEN_UP:
		case GEN_DOWN:
		case GEN_LEFT:
		case GEN_RIGHT:
		case GEN_REPROD:
		case GEN_DIE:
			return gen_strings[gen];
	}

	return gen_strings[GEN_MAX];
}


static void cell_generate_cell(int initial, cell_t *cell)
{
	int i;

	// srand(time(NULL));

	if (initial)
		cell->view = (rand()%20)? 0:ACS_DIAMOND;
	else
		cell->view = ACS_DIAMOND;

	if (cell->view)
	{
		cell->ttl = rand()%TTL_DEF + 10;
		for (i = 0; i < GENOM_SIZE; i++)
		{
			cell->genom[i] = rand()%GEN_MAX;
		}
		population++;
	}
}

static void main_field_init(void)
{
	int i, j;
	int ig;

	srand(time(NULL));

	for (i = 0; i < MAIN_FIELD_HEIGHT; i++)
	{
		memset(main_field[i], 0, sizeof(main_field[i]));
		for (j = 0; j < MAIN_FIELD_WIDTH; j++)
		{
			cell_generate_cell(1, &main_field[i][j]);
		}
	}
}

static gen_t *key_proc_up(void)
{
	// mvprintw(0, 0, "UP");
}

static gen_t *key_proc_down(void)
{
	// mvprintw(0, 0, "DOWN");
}

static gen_t *key_proc_left(void)
{
	// mvprintw(0, 0, "LEFT");
}

static gen_t *key_proc_right(void)
{
	// mvprintw(0, 0, "RIGHT");
}

static void proc_cell_ttl(cell_t *cell)
{
	if (++cell->gen >= GENOM_SIZE)
		cell->gen = 0;

	if (--cell->ttl < 0)
	{
		// memset(cell, 0, sizeof(cell_t));
		// cell->gen = 0;
		// continue;
	}
}

static void key_proc_enter(int x, int y)
{
	int i, j;
	int ig, neigh;
	cell_t *cell;

	for (i = 0; i < MAIN_FIELD_HEIGHT; i++)
	{
		for (j = 0; j < MAIN_FIELD_WIDTH; j++)
		{
			if (!main_field[i][j].view)
				continue;

			if (main_field[i][j].ttl <= 0)
			{
				memset(&main_field[i][j], 0, sizeof(main_field[i][j]));
			}

			ig = main_field[i][j].gen;
			proc_cell_ttl(&main_field[i][j]);

			switch(main_field[i][j].genom[ig])
			{
				case GEN_NONE:
					main_field[i][j].step_is_done++;
					break;
				case GEN_UP:
					if (i && !main_field[i-1][j].view)
					{
						main_field[i][j].step_is_done++;
						memcpy(&main_field[i-1][j], &main_field[i][j], sizeof(main_field[i][j]));
						memset(&main_field[i][j], 0, sizeof(main_field[i][j]));
					}
					break;
				case GEN_DOWN:
					if ((i < MAIN_FIELD_HEIGHT-1) && !main_field[i+1][j].view)
					{
						main_field[i][j].step_is_done++;
						memcpy(&main_field[i+1][j], &main_field[i][j], sizeof(main_field[i][j]));
						memset(&main_field[i][j], 0, sizeof(main_field[i][j]));
					}
					break;
				case GEN_LEFT:
					if (j && !main_field[i][j-1].view)
					{
						main_field[i][j].step_is_done++;
						memcpy(&main_field[i][j-1], &main_field[i][j], sizeof(main_field[i][j]));
						memset(&main_field[i][j], 0, sizeof(main_field[i][j]));
					}
					break;
				case GEN_RIGHT:
					if ((j < MAIN_FIELD_WIDTH-1) && !main_field[i][j+1].view)
					{
						main_field[i][j].step_is_done++;
						memcpy(&main_field[i][j+1], &main_field[i][j], sizeof(main_field[i][j]));
						memset(&main_field[i][j], 0, sizeof(main_field[i][j]));
					}
					break;
				case GEN_REPROD:
					if (i & j && (i < MAIN_FIELD_HEIGHT-1 && j < MAIN_FIELD_WIDTH-1))
					{
						neigh = 0;
						if (main_field[i-1][j].view)
							neigh++;
						else
							cell = &main_field[i-1][j];

						if (main_field[i+1][j].view)
							neigh++;
						else
							cell = &main_field[i+1][j];

						if (main_field[i][j-1].view)
							neigh++;
						else
							cell = &main_field[i][j-1];

						if (main_field[i][j+1].view)
							neigh++;
						else
							cell = &main_field[i][j+1];

						if (main_field[i-1][j-1].view)
							neigh++;
						else
							cell = &main_field[i-1][j-1];

						if (main_field[i+1][j+1].view)
							neigh++;
						else
							cell = &main_field[i+1][j+1];

						if (main_field[i+1][j-1].view)
							neigh++;
						else
							cell = &main_field[i+1][j-1];

						if (main_field[i-1][j+1].view)
							neigh++;
						else
							cell = &main_field[i-1][j+1];
					}

					if (neigh < 1 || neigh > 4)
					{
						memset(&main_field[i][j], 0, sizeof(main_field[i][j]));
						continue;
					}
					else
					{
						cell_generate_cell(0, cell);
						main_field[i][j].ttl++;
					}
					break;
				case GEN_DIE:
					memset(&main_field[i][j], 0, sizeof(main_field[i][j]));
					break;
			}
		}
	}

	population = 0;
	for (i = 0; i < MAIN_FIELD_HEIGHT; i++)
		for (j = 0; j < MAIN_FIELD_WIDTH; j++)
		{
			main_field[i][j].step_is_done = 0;
			if (main_field[i][j].view)
				population++;
		}
}

static void field_print(void)
{
	int i, j;
	int color;

	for (i = 0; i < MAIN_FIELD_HEIGHT; i++)
	{
		for (j = 0; j < MAIN_FIELD_WIDTH; j++)
		{
			if (main_field[i][j].view)
			{
				color = TTL_IS_GOOD;

				if (main_field[i][j].ttl <= (int)(TTL_DEF/3))
				{
					color = TTL_IS_BAD;
				} else
				if ((main_field[i][j].ttl > (int)(TTL_DEF/3)) && (main_field[i][j].ttl <= (int)(TTL_DEF/3)*2))
				{
					color = TTL_IS_NORMAL;
				}

				attron (COLOR_PAIR (color));
				mvaddch(i, j, main_field[i][j].view);
				attroff (COLOR_PAIR (color));
			}
		}
	}
}

static void field_print_genom(int x, int y)
{
	int i, j, step = 0;
	char *gstr;

	if (main_field[y][x].view)
	{
		mvprintw(3, MAIN_FIELD_WIDTH + 5, "ttl = %d", main_field[y][x].ttl);
		for (i = 0; i < GENOM_SIZE; i++)
		{
			step = main_field[y][x].gen == i;
			gstr = genom_str(main_field[y][x].genom[i]);
			mvprintw(4 + i%GENOM_SIZE, MAIN_FIELD_WIDTH + 5, "%s %s", gstr, step? "<---":"");
		}
	}
}

int main(int argc, char *argv[])
{
	int c = 0, quit = 0, enter = 0;
	int x = 0, y = 0;
	int delay = 50000;
	int max_population = 0;

	initscr();
	noecho();
	curs_set(FALSE);
	keypad(stdscr, TRUE);
	start_color();
	clear();
	main_field_init();

	init_pair(TTL_IS_GOOD, COLOR_GREEN, COLOR_BLACK);
	init_pair(TTL_IS_NORMAL, COLOR_YELLOW, COLOR_BLACK);
	init_pair(TTL_IS_BAD, COLOR_RED, COLOR_BLACK);

	while(!quit && population)
	{
		field_print();
		mvprintw(0, MAIN_FIELD_WIDTH + 5, "iteration %d", enter);
		mvprintw(1, MAIN_FIELD_WIDTH + 5, "population %d", population);
		mvprintw(2, MAIN_FIELD_WIDTH + 5, "fps %d (%d)", 1000000/delay, delay);
		mvaddch(y, x, ACS_CKBOARD);
		field_print_genom(x, y);

		refresh();
#if 0
		if (population <= 200)
			delay = 25000;
		else if (population > 200 && population <= 500)
			delay = 50000;
		else
			delay = 100000;

		if (population > max_population)
			max_population = population;

		usleep(delay);
		key_proc_enter(x, y);
		enter++;
		clear();
#else
		c = getch();
		clear();
		switch(c)
		{
			case KEY_UP:
				if (--y <= 0)
					y = 0;
				break;
			case KEY_DOWN:
				if (++y >= MAIN_FIELD_HEIGHT)
					y = MAIN_FIELD_HEIGHT;
				break;
			case KEY_LEFT:
				if (--x <= 0)
					x = 0;
				break;
			case KEY_RIGHT:
				if (++x >= MAIN_FIELD_WIDTH)
					x = MAIN_FIELD_WIDTH;
				break;
			case ' ':
				key_proc_enter(x, y);
				enter++;
				break;
			case 'q':
				quit++;
				break;
			default:
				break;
		}
#endif
	}

	endwin();

	printf("--> iteration %d <--\n", enter);
	printf("--> max population %d <--\n", max_population);

	return 0;
}