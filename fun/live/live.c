#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <time.h>

// #define MAIN_FIELD_WIDTH 100 /* ширина */
// #define MAIN_FIELD_HEIGHT 40 /* высота */
#define MAIN_FIELD_WIDTH 40 /* ширина */
#define MAIN_FIELD_HEIGHT 20 /* высота */

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
	int ID;

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

static cell_t main_field[MAIN_FIELD_HEIGHT][MAIN_FIELD_WIDTH];
static int population = 0;

static void print_cell(int i, int j);

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
		default:
			break;
	}

	return gen_strings[GEN_MAX];
}


static void cell_generate_cell(int initial, cell_t *cell)
{
	int i;
	static int ID = 0;

	if (initial)
		cell->view = (rand()%20)? 0:ACS_DIAMOND;
	else
		cell->view = ACS_DIAMOND;

	if (cell->view)
	{
		cell->ID = ID++;
		cell->ttl = rand()%TTL_DEF + 10;
		for (i = 0; i < GENOM_SIZE; i++)
		{
			cell->genom[i] = rand()%GEN_MAX;
			if (cell->genom[i] == GEN_DIE)
				--i;
		}
		population++;
	}
}

static void main_field_init(void)
{
	int i, j;

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

static void proc_cell_ttl(cell_t *cell)
{
	if (++cell->gen >= GENOM_SIZE)
		cell->gen = 0;

	--cell->ttl;
}

static void proc_gen_none(int x, int y)
{
	main_field[x][y].step_is_done++;
}

static void proc_gen_up(int x, int y)
{
	if (!x || main_field[x-1][y].view)
		return;

	main_field[x][y].step_is_done++;
	memcpy(&main_field[x-1][y], &main_field[x][y], sizeof(main_field[x][y]));
	memset(&main_field[x][y], 0, sizeof(main_field[x][y]));

	print_cell(x-1, y);
	print_cell(x, y);
}

static void proc_gen_down(int x, int y)
{
	if ((x >= MAIN_FIELD_HEIGHT-1) || main_field[x+1][y].view)
		return;

	main_field[x][y].step_is_done++;
	memcpy(&main_field[x+1][y], &main_field[x][y], sizeof(main_field[x][y]));
	memset(&main_field[x][y], 0, sizeof(main_field[x][y]));

	print_cell(x+1, y);
	print_cell(x, y);
}

static void proc_gen_left(int x, int y)
{
	if (!y || main_field[x][y-1].view)
		return;

	main_field[x][y].step_is_done++;
	memcpy(&main_field[x][y-1], &main_field[x][y], sizeof(main_field[x][y]));
	memset(&main_field[x][y], 0, sizeof(main_field[x][y]));

	print_cell(x, y-1);
	print_cell(x, y);
}

static void proc_gen_right(int x, int y)
{
	if ((y >= MAIN_FIELD_WIDTH-1) || main_field[x][y+1].view)
		return;

	main_field[x][y].step_is_done++;
	memcpy(&main_field[x][y+1], &main_field[x][y], sizeof(main_field[x][y]));
	memset(&main_field[x][y], 0, sizeof(main_field[x][y]));

	print_cell(x, y+1);
	print_cell(x, y);
}

static void proc_gen_reprod(int x, int y)
{
	cell_t *cell = NULL;
	int neigh = 0;
	int i = 0, j = 0;

	if (x & y && (x < MAIN_FIELD_HEIGHT-1 && y < MAIN_FIELD_WIDTH-1))
	{
		if (main_field[x-1][y].view)
			neigh++;
		else
		{
			cell = &main_field[x-1][y];
			i = x-1; j = y;
		}

		if (main_field[x+1][y].view)
			neigh++;
		else
		{
			cell = &main_field[x+1][y];
			i = x+1; j = y;
		}

		if (main_field[x][y-1].view)
			neigh++;
		else
		{
			cell = &main_field[x][y-1];
			i = x; j = y-1;
		}

		if (main_field[x][y+1].view)
			neigh++;
		else
		{
			cell = &main_field[x][y+1];
			i = x; j = y+1;
		}

		if (main_field[x-1][y-1].view)
			neigh++;
		else
		{
			cell = &main_field[x-1][y-1];
			i = x-1; j = y-1;
		}

		if (main_field[x+1][y+1].view)
			neigh++;
		else
		{
			cell = &main_field[x+1][y+1];
			i = x+1; j = y+1;
		}

		if (main_field[x+1][y-1].view)
			neigh++;
		else
		{
			cell = &main_field[x+1][y-1];
			i = x+1; j = y-1;
		}

		if (main_field[x-1][y+1].view)
			neigh++;
		else
		{
			cell = &main_field[x-1][y+1];
			i = x-1; j = y+1;
		}
	}

	if (neigh < 1 || neigh > 4)
	{
		memset(&main_field[x][y], 0, sizeof(main_field[x][y]));
		// return;
	} else
	if (cell)
	{
		cell_generate_cell(0, cell);

		print_cell(i, j);
		main_field[x][y].ttl++;
	}

	print_cell(x, y);
}

static void proc_gen_die(int x, int y)
{
	memset(&main_field[x][y], 0, sizeof(main_field[x][y]));
}

static void key_proc_enter(int x, int y)
{
	int i, j;
	int ig;

	for (i = 0; i < MAIN_FIELD_HEIGHT; i++)
	{
		for (j = 0; j < MAIN_FIELD_WIDTH; j++)
		{
			if (!main_field[i][j].view)
				continue;

			if (main_field[i][j].step_is_done)
				continue;

			if (main_field[i][j].ttl <= 0)
			{
				memset(&main_field[i][j], 0, sizeof(main_field[i][j]));
				print_cell(i, j);
				continue;
			}

			ig = main_field[i][j].gen;
			proc_cell_ttl(&main_field[i][j]);

			switch(main_field[i][j].genom[ig])
			{
				case GEN_NONE:
					proc_gen_none(i, j);
					break;
				case GEN_UP:
					proc_gen_up(i, j);
					break;
				case GEN_DOWN:
					proc_gen_down(i, j);
					break;
				case GEN_LEFT:
					proc_gen_left(i, j);
					break;
				case GEN_RIGHT:
					proc_gen_right(i, j);
					break;
				case GEN_REPROD:
					proc_gen_reprod(i, j);
					break;
				case GEN_DIE:
					proc_gen_die(i, j);
					break;

				default:
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

static void print_cell(int i, int j)
{
	int color;

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
	else
	{
		mvaddch(i, j, ' ');
	}
}

static void field_print(void)
{
	int i, j;

	for (i = 0; i < MAIN_FIELD_HEIGHT; i++)
		for (j = 0; j < MAIN_FIELD_WIDTH; j++)
			print_cell(i, j);
}

static void field_print_genom(int x, int y)
{
	int i, step = 0;
	char *gstr;

	if (main_field[y][x].view)
	{
		mvprintw(9, MAIN_FIELD_WIDTH + 5, "ID %d", main_field[y][x].ID);
		mvprintw(10, MAIN_FIELD_WIDTH + 5, "ttl %d", main_field[y][x].ttl);
		for (i = 0; i < GENOM_SIZE; i++)
		{
			step = main_field[y][x].gen == i;
			gstr = genom_str(main_field[y][x].genom[i]);
			mvprintw(11 + i%GENOM_SIZE, MAIN_FIELD_WIDTH + 5, "%s %s", gstr, step? "<---":"");
		}
	}
	else
	{
		mvprintw(9, MAIN_FIELD_WIDTH + 5, "                   ");
		mvprintw(10, MAIN_FIELD_WIDTH + 5, "                   ");
		for (i = 0; i < GENOM_SIZE; i++)
			mvprintw(11 + i%GENOM_SIZE, MAIN_FIELD_WIDTH + 5, "                   ");
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

	field_print();
	mvaddch(y, x, ACS_CKBOARD);

	while(!quit && population)
	{
		mvprintw(0, MAIN_FIELD_WIDTH + 5, "iteration %d", enter);
		mvprintw(1, MAIN_FIELD_WIDTH + 5, "population %d", population);
		mvprintw(2, MAIN_FIELD_WIDTH + 5, "fps %d (%d)", 1000000/delay, delay);
		field_print_genom(x, y);
		// mvaddch(y, x, ACS_CKBOARD);

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

		// field_print();
		// mvprintw(0, MAIN_FIELD_WIDTH + 5, "iteration %d", enter);
		// mvprintw(1, MAIN_FIELD_WIDTH + 5, "population %d", population);
		// mvprintw(2, MAIN_FIELD_WIDTH + 5, "fps %d (%d)", 1000000/delay, delay);
		// field_print_genom(x, y);

		switch(c)
		{
			case KEY_UP:
				print_cell(y, x);
				if (--y <= 0)
					y = 0;
				mvaddch(y, x, ACS_CKBOARD);
				break;
			case KEY_DOWN:
				print_cell(y, x);
				if (++y >= MAIN_FIELD_HEIGHT)
					y = MAIN_FIELD_HEIGHT - 1;
				mvaddch(y, x, ACS_CKBOARD);
				break;
			case KEY_LEFT:
				print_cell(y, x);
				if (--x <= 0)
					x = 0;
				mvaddch(y, x, ACS_CKBOARD);
				break;
			case KEY_RIGHT:
				print_cell(y, x);
				if (++x >= MAIN_FIELD_WIDTH)
					x = MAIN_FIELD_WIDTH - 1;
				mvaddch(y, x, ACS_CKBOARD);
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
		// clear();
#endif
	}

	endwin();

	printf("--> iteration %d <--\n", enter);
	printf("--> max population %d <--\n", max_population);

	return 0;
}