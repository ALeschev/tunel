#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

#define MAIN_FIELD_WIDTH 100 /* ширина */
#define MAIN_FIELD_HEIGHT 40 /* высота */
#define GENOM_SIZE 4

typedef enum
{
	GEN_NONE,

	GEN_UP,
	GEN_DOWN,
	GEN_LEFT,
	GEN_RIGHT,

	GEN_REPROD,

	GEN_MAX
} gen_t;

typedef struct
{
	gen_t genom[GENOM_SIZE];
	int gen_step;
	int step_is_done;
	int ttl;
	// color
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

	"UNKN"
};

static cell_t main_field[MAIN_FIELD_HEIGHT][MAIN_FIELD_WIDTH] = {{0}, {0}};

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
			return gen_strings[gen];
	}

	return gen_strings[GEN_MAX];
}

static void cell_generate_cell(cell_t *cell)
{
	int i;

	// srand(time(NULL));

	cell->view = (rand()%50)? 0:ACS_DIAMOND;
	if (cell->view)
	{
		cell->ttl = 50;
		for (i = 0; i < GENOM_SIZE; i++)
		{
			cell->genom[i] = rand()%GEN_MAX;
		}
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
			cell_generate_cell(&main_field[i][j]);
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

			if (main_field[i][j].step_is_done)
			{
				main_field[i][j].step_is_done = 0;
				continue;
			}

			if (++main_field[i][j].gen_step >= GENOM_SIZE)
				main_field[i][j].gen_step = 0;

			ig = main_field[i][j].gen_step;



			// mvprintw(10, MAIN_FIELD_WIDTH + 5, "(%d, %d) %s", i,j,genom_str(main_field[i][j].genom[ig]));

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
						if (main_field[i][j-1].view)
							neigh++;
						else
							cell = &main_field[i][j-1];
						if (main_field[i+1][j].view)
							neigh++;
						else
							cell = &main_field[i+1][j];
						if (main_field[i][j+1].view)
							neigh++;
						else
							cell = &main_field[i][j+1];

						if (neigh < 1 || neigh > 3)
							memset(&main_field[i][j], 0, sizeof(main_field[i][j]));
						else
							cell_generate_cell(cell);
					}
					break;
			}

			if (--main_field[i][j].ttl <= 0)
				memset(&main_field[i][j], 0, sizeof(main_field[i][j]));
		}
	}

	for (i = 0; i < MAIN_FIELD_HEIGHT; i++)
		for (j = 0; j < MAIN_FIELD_WIDTH; j++)
			main_field[i][j].step_is_done = 0;
}

static void field_print(void)
{
	int i, j;

	for (i = 0; i < MAIN_FIELD_HEIGHT; i++)
	{
		for (j = 0; j < MAIN_FIELD_WIDTH; j++)
		{
			if (main_field[i][j].view)
				mvaddch(i, j, main_field[i][j].view);
		}
	}
}

static void field_print_genom(int x, int y)
{
	int i, j, step = 0;
	char *gstr;

	mvprintw(3, MAIN_FIELD_WIDTH + 5, "ttl = %d", main_field[y][x].ttl);

	for (i = 0; i < GENOM_SIZE; i++)
	{
		step = main_field[y][x].gen_step == i;
		gstr = genom_str(main_field[y][x].genom[i]);
		mvprintw(4 + i%GENOM_SIZE, MAIN_FIELD_WIDTH + 5, "%s %s", gstr, step? "<---":"");
	}
}

int main(int argc, char *argv[])
{
	int c = 0, quit = 0, enter = 0;
	int x = 0, y = 0;

	initscr();
	noecho();
	curs_set(FALSE);
	keypad(stdscr, TRUE);
	clear();
	main_field_init();

	while(!quit)
	{
		field_print();
		mvaddch(y, x, ACS_CKBOARD);
		mvprintw(0, MAIN_FIELD_WIDTH + 5, "x = %d", x);
		mvprintw(1, MAIN_FIELD_WIDTH + 5, "y = %d", y);
		mvprintw(2, MAIN_FIELD_WIDTH + 5, "e = %d", enter);
		field_print_genom(x, y);

		refresh();
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
	}

	endwin();

	return 0;
}