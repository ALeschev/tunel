#include <curses.h>

int main(void)
{
    initscr();
    start_color();

    init_pair(1, COLOR_BLACK, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    init_pair(5, COLOR_BLUE, COLOR_BLACK);
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(7, COLOR_CYAN, COLOR_BLACK);
    init_pair(8, COLOR_WHITE, COLOR_BLACK);

    attron(COLOR_PAIR(1));
    printw("init_pair(1, COLOR_BLACK, COLOR_BLACK);\n");

    attron(COLOR_PAIR(2));
    printw("init_pair(2, COLOR_RED, COLOR_BLACK);\n");

    attron(COLOR_PAIR(3));
    printw("init_pair(3, COLOR_GREEN, COLOR_BLACK);\n");

    attron(COLOR_PAIR(4));
    printw("init_pair(4, COLOR_YELLOW, COLOR_BLACK);\n");

    attron(COLOR_PAIR(5));
    printw("init_pair(5, COLOR_BLUE, COLOR_BLACK);\n");

    attron(COLOR_PAIR(6));
    printw("init_pair(6, COLOR_MAGENTA, COLOR_BLACK);\n");

    attron(COLOR_PAIR(7));
    printw("init_pair(7, COLOR_CYAN, COLOR_BLACK);\n");

    attron(COLOR_PAIR(8));
    printw("init_pair(8, COLOR_WHITE, COLOR_BLACK);\n");


    refresh();

    getch();

    endwin();

    return 0;
}