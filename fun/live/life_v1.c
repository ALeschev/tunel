#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>

#define FIELD_WIDTH 80 /* ширина */
#define FIELD_HEIGHT 40 /* высота */

#define TTL_IS_GOOD 1
#define TTL_IS_NORMAL 2
#define TTL_IS_BAD 3
#define PREDATOR 4

typedef struct
{
    int new;
    int view;
} cell_t;

static cell_t main_field[FIELD_HEIGHT][FIELD_WIDTH];

static int debug_mode = 0;
static int quit = 0;
static int population = 10;

static inline void print_cell(int y, int x);

static inline void cell_generate(int initial, int y, int x)
{
    if (initial)
        main_field[y][x].view = (rand()%5)? 0:ACS_DIAMOND;
    else
        main_field[y][x].view = ACS_DIAMOND;

    if (main_field[y][x].view)
    {
        population++;

        if (!initial)
            main_field[y][x].new++;
    }
}

static inline void cell_debug_put(int y, int x)
{
    cell_generate(0, y, x);
    main_field[y][x].new = 0;
}

static inline void cell_die(int y, int x)
{
    if (main_field[y][x].view)
    {
        main_field[y][x].view = 0;
        main_field[y][x].new = 1;
        population--;
    }
}

static inline void cell_debug_die(int y, int x)
{
    cell_die(y, x);
    main_field[y][x].new = 0;
}

static void main_field_init(void)
{
    int y, x;

    srand(time(NULL));

    for (y = 0; y < FIELD_HEIGHT; y++)
    {
        memset(main_field[y], 0, sizeof(main_field[y]));
        for (x = 0; x < FIELD_WIDTH; x++)
        {
            cell_generate(1, y, x);
        }
    }
}

static inline int cell_die_new(int y, int x)
{
    return !main_field[y][x].view && main_field[y][x].new;
}

static inline int cell_life_new(int y, int x)
{
    return main_field[y][x].view && !main_field[y][x].new;
}

static int get_neighbours(int y, int x)
{
    int neighbours = 0;


    if (y && x && (x != FIELD_WIDTH-1) && (y != FIELD_HEIGHT-1))
    {
        /**
         * #|#|#
         * #|@|#
         * #|#|#
         */

        /**
         * *|#|#
         * #|@|#
         * #|#|#
         */
        if (cell_life_new(y-1, x-1) || cell_die_new(y-1, x-1))
            neighbours++;

        /**
         * #|*|#
         * #|@|#
         * #|#|#
         */
        if (cell_life_new(y-1, x) || cell_die_new(y-1, x))
            neighbours++;

        /**
         * #|#|*
         * #|@|#
         * #|#|#
         */
        if (cell_life_new(y-1, x+1) || cell_die_new(y-1, x+1))
            neighbours++;

        /**
         * #|#|#
         * *|@|#
         * #|#|#
         */
        if (cell_life_new(y, x-1) || cell_die_new(y, x-1))
            neighbours++;

        /**
         * #|#|#
         * #|@|*
         * #|#|#
         */
        if (cell_life_new(y, x+1) || cell_die_new(y, x+1))
            neighbours++;

        /**
         * #|#|#
         * #|@|#
         * *|#|#
         */
        if (cell_life_new(y+1, x-1) || cell_die_new(y+1, x-1))
            neighbours++;

        /**
         * #|#|#
         * #|@|#
         * #|*|#
         */
        if (cell_life_new(y+1, x) || cell_die_new(y+1, x))
            neighbours++;

        /**
         * #|#|#
         * #|@|#
         * #|#|*
         */
        if (cell_life_new(y+1, x+1) || cell_die_new(y+1, x+1))
            neighbours++;

    } else
    if (!y)
    {
        /**
         * #|@|#
         * #|#|#
         */

        if (x)
        {
            if (cell_life_new(y, x-1) || cell_die_new(y, x-1))
                neighbours++;

            if (cell_life_new(y+1, x-1) || cell_die_new(y+1, x-1))
                neighbours++;
        }

        if (x != FIELD_WIDTH-1)
        {
            if (cell_life_new(y, x+1) || cell_die_new(y, x+1))
                neighbours++;

            if (cell_life_new(y+1, x+1) || cell_die_new(y+1, x+1))
                neighbours++;
        }

        if (cell_life_new(y+1, x) || cell_die_new(y+1, x))
            neighbours++;
    } else
    if (!x)
    {
        /**
         * #|#
         * @|#
         * #|#
         */

        if (cell_life_new(y-1, x) || cell_die_new(y-1, x))
            neighbours++;

        if (cell_life_new(y-1, x+1) || cell_die_new(y-1, x+1))
            neighbours++;

        if (cell_life_new(y, x+1) || cell_die_new(y, x+1))
            neighbours++;

        if (y != FIELD_HEIGHT-1)
        {
            if (cell_life_new(y+1, x) || cell_die_new(y+1, x))
                neighbours++;

            if (cell_life_new(y+1, x+1) || cell_die_new(y+1, x+1))
                neighbours++;
        }
    } else
    if (x == FIELD_WIDTH-1)
    {
        /**
         * #|#
         * #|@
         * #|#
         */

        if (cell_life_new(y-1, x) || cell_die_new(y-1, x))
            neighbours++;

        if (cell_life_new(y-1, x-1) || cell_die_new(y-1, x-1))
            neighbours++;

        if (cell_life_new(y, x-1) || cell_die_new(y, x-1))
            neighbours++;

        if (y != FIELD_HEIGHT-1)
        {
            if (cell_life_new(y+1, x-1) || cell_die_new(y+1, x-1))
                neighbours++;

            if (cell_life_new(y+1, x) || cell_die_new(y+1, x))
                neighbours++;
        }
    } else
    if (y == FIELD_HEIGHT-1)
    {
        /**
         * #|#|#
         * #|@|#
         */

        if (cell_life_new(y-1, x-1) || cell_die_new(y-1, x-1))
            neighbours++;

        if (cell_life_new(y-1, x) || cell_die_new(y-1, x))
            neighbours++;

        if (cell_life_new(y-1, x+1) || cell_die_new(y-1, x+1))
            neighbours++;

        if (cell_life_new(y, x-1) || cell_die_new(y, x-1))
            neighbours++;

        if (cell_life_new(y, x+1) || cell_die_new(y, x+1))
            neighbours++;
    }

    return neighbours;
}

static void key_proc_enter(void)
{
    int y, x;
    int neighbours = 0;

    for (y = 0; y < FIELD_HEIGHT; y++)
        for (x = 0; x < FIELD_WIDTH; x++)
            main_field[y][x].new = 0;

    for (y = 0; y < FIELD_HEIGHT; y++)
    {
        for (x = 0; x < FIELD_WIDTH; x++)
        {
            neighbours = get_neighbours(y, x);

            if (main_field[y][x].view)
            {
                if (neighbours < 2 || 3 < neighbours)
                {
                    cell_die(y, x);
                }
            }
            else if (neighbours == 3)
            {
                cell_generate(0, y, x);
            }

            print_cell(y, x);
        }
    }
}

static inline void print_cell(int y, int x)
{
    if (main_field[y][x].view)
    {
        attron (COLOR_PAIR (TTL_IS_GOOD));
        mvaddch(y, x, main_field[y][x].view);
        attroff (COLOR_PAIR (TTL_IS_GOOD));
    }
    else
    {
        mvaddch(y, x, ' ');
    }
}

static void field_print(void)
{
    int y, x;

    for (y = 0; y < FIELD_HEIGHT; y++)
        for (x = 0; x < FIELD_WIDTH; x++)
            print_cell(y, x);
}

static void field_dbg_info(int y, int x)
{
    mvprintw(9, FIELD_WIDTH + 5,  "i  iterate");
    mvprintw(10, FIELD_WIDTH + 5, "+  add cell");
    mvprintw(11, FIELD_WIDTH + 5, "-  del cell");
    mvprintw(12, FIELD_WIDTH + 5, "o  debug mode off");
    mvprintw(13, FIELD_WIDTH + 5, "q  quit");

    mvprintw(15, FIELD_WIDTH + 5, "                     ");
    mvprintw(16, FIELD_WIDTH + 5, "                    ");

    // if (main_field[y][x].view)
    {
        mvprintw(15, FIELD_WIDTH + 5, "neighbours %d", get_neighbours(y, x));
        mvprintw(16, FIELD_WIDTH + 5, "new %d", main_field[y][x].new);
    }
    // else
    // {
    //     mvprintw(9, FIELD_WIDTH + 5, "                     ");
    //     mvprintw(10, FIELD_WIDTH + 5, "                    ");
    // }
}

static inline void time_start(struct timeval *tv1)
{
    gettimeofday(tv1, NULL);
}

static inline unsigned long time_stop(struct timeval *tv1)
{ 
    struct timeval tv2;
    struct timeval dtv;

    gettimeofday(&tv2, NULL);

    dtv.tv_sec= tv2.tv_sec -tv1->tv_sec;

    dtv.tv_usec=tv2.tv_usec-tv1->tv_usec;

    if(dtv.tv_usec < 0)
    {
        dtv.tv_sec--;
        dtv.tv_usec+=1000000;
    }

    return dtv.tv_sec*1000000+dtv.tv_usec;
}

void signal_handler(int sig)
{
    quit++;
}

int main(int argc, char *argv[])
{
    int c = 0, enter = 0;
    int x = 0, y = 0;
    int delay = 50000;
    int max_population = 0;
    struct timeval tv1;

    if (argc >= 2 && !strcmp(argv[1], "d"))
        debug_mode++;

    signal(SIGINT, signal_handler);

    initscr();
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    start_color();
    clear();

    // if (!debug_mode)
        main_field_init();

    init_pair(TTL_IS_GOOD, COLOR_GREEN, COLOR_BLACK);
    init_pair(TTL_IS_NORMAL, COLOR_YELLOW, COLOR_BLACK);
    init_pair(TTL_IS_BAD, COLOR_MAGENTA, COLOR_BLACK);

    init_pair(PREDATOR, COLOR_RED, COLOR_BLACK);

    field_print();

    if (debug_mode)
        mvaddch(y, x, ACS_CKBOARD);

    delay = 100000;

    if (debug_mode)
        mvprintw(0, FIELD_WIDTH + 5, "field %dx%d (%d cells)", FIELD_WIDTH, FIELD_HEIGHT, FIELD_HEIGHT*FIELD_WIDTH);

    if (!debug_mode)
        getch();

    time_start(&tv1);

    while(!quit && population)
    {
        // if (debug_mode)
        {
            mvprintw(1, FIELD_WIDTH + 5, "iteration %d", enter);
            mvprintw(2, FIELD_WIDTH + 5, "population %d      ", population);
            mvprintw(3, FIELD_WIDTH + 5, "fps %d (%dms)   ", 1000000/delay, delay);

            if (debug_mode)
                field_dbg_info(y, x);
        }
        // mvaddch(y, x, ACS_CKBOARD);

        refresh();

        if (population > max_population)
            max_population = population;

        if (!debug_mode)
        {
            // delay = ((double)population / (double)(FIELD_HEIGHT*FIELD_WIDTH)) * 500000 + 10000;
            delay = 100000;
            usleep(delay);
            key_proc_enter();
            enter++;
        }
        else
        {
            c = getch();

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
                    if (++y >= FIELD_HEIGHT)
                        y = FIELD_HEIGHT - 1;
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
                    if (++x >= FIELD_WIDTH)
                        x = FIELD_WIDTH - 1;
                    mvaddch(y, x, ACS_CKBOARD);
                    break;
                case 'i':
                    key_proc_enter();
                    enter++;
                    break;

                case '+':
                    cell_debug_put(y, x);
                    break;

                case '-':
                    cell_debug_die(y, x);
                    break;

                case 'o':
                    debug_mode = 0;
                    break;

                case 'q':
                    quit++;
                    break;
                default:
                    break;
            }
        }

    }

    // if (!debug_mode)
    //     getch();

    endwin();

    printf("--> iteration %d <--\n", enter);
    printf("--> max population %d <--\n", max_population);
    printf("--> live time is ~%lus <--\n", time_stop(&tv1)/1000000);

    return 0;
}