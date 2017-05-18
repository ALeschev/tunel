#include <ncurses.h>
#include <string.h>

#define WIDTH 100
#define HEIGHT 20 

enum box_name
{
    eBNAME_0 = 0,
    eBNAME_1 = 1,
    eBNAME_2 = 2,
    eBNAME_3 = 3,
    eBNAME_4 = 4,
    eBNAME_5 = 5,
    eBNAME_6 = 6,
    eBNAME_7 = 7,
    eBNAME_8 = 8,
    eBNAME_9 = 9,
    eBNAME_STAR = 10,
    eBNAME_HASH = 11,

    eBNAME_MAX
};

typedef struct
{
    int x;
    int y;
} point_t;

typedef struct
{
    int height;
    int width;
} form_t;

typedef struct
{
    char name[64];
    int name_len;
    point_t point;
    point_t center;
    form_t form;

    int highlight;
} box_t;

box_t field[eBNAME_MAX];

void box_get_center(box_t *box, int content_len, point_t *point)
{
    int x_center = box->point.x + box->form.width/2 - content_len/2;

    if (x_center < box->point.x)
    {
        memcpy(point, &box->center, sizeof(point_t));
    }
    else
    {
        point->x = x_center;
        point->y = box->point.y + box->form.height/2;
    }
}

void box_draw(point_t *point, form_t *form, bool clear)
{   
    int i, j;

    if(!clear)
    {
        mvaddch(point->y, point->x, '+');
        mvaddch(point->y, point->x + form->width, '+');
        mvaddch(point->y + form->height, point->x, '+');
        mvaddch(point->y + form->height, point->x + form->width, '+');
        mvhline(point->y, point->x + 1, '-', form->width - 1);
        mvhline(point->y + form->height, point->x + 1, '-', form->width - 1);
        mvvline(point->y + 1, point->x, '|', form->height - 1);
        mvvline(point->y + 1, point->x + form->width, '|', form->height - 1);
    }
    else
    {
        for(j = point->y; j <= point->y + form->height; ++j)
            for(i = point->x; i <= point->x + form->width; ++i)
                mvaddch(j, i, ' ');
    }
}

void init_pads(void)
{
    int i;
    int x = 1, y = 1;
    int width = 4, height = 2;

    memset(field, 0, sizeof(field));

    for (i = 0; i < 12; i++)
    {
        if (i == eBNAME_STAR || i == eBNAME_HASH || i == eBNAME_0)
        {
            field[i].form.height = height;
            field[i].form.width = width;
            field[i].name_len = 1;
            continue;
        }

        field[i].point.x = x;
        field[i].point.y = y;
        field[i].form.height = height;
        field[i].form.width = width;
        field[i].name[0] = i + 0x30;
        field[i].name_len = 1;

        box_get_center(&field[i], field[i].name_len, &field[i].center);

        if (!(i % 3))
        {
            y += height;
            x = 1;
        } 
        else
        {
            x += width;
        }

        box_draw(&field[i].point, &field[i].form, 0);
    }


    field[eBNAME_STAR].point.x = 1;
    field[eBNAME_STAR].point.y = 1 + height * 3;
    field[eBNAME_STAR].name[0] = '*';
    box_get_center(&field[eBNAME_STAR], field[eBNAME_STAR].name_len, &field[eBNAME_STAR].center);
    box_draw(&field[eBNAME_STAR].point, &field[eBNAME_STAR].form, 0);

    field[eBNAME_HASH].point.x = 1 + width * 2;
    field[eBNAME_HASH].point.y = 1 + height * 3;
    field[eBNAME_HASH].name[0] = '#';
    box_get_center(&field[eBNAME_HASH], field[eBNAME_HASH].name_len, &field[eBNAME_HASH].center);
    box_draw(&field[eBNAME_HASH].point, &field[eBNAME_HASH].form, 0);

    field[eBNAME_0].point.x = 1 + width * 1;
    field[eBNAME_0].point.y = 1 + height * 3;
    field[eBNAME_0].name[0] = '0';
    box_get_center(&field[eBNAME_0], field[eBNAME_0].name_len, &field[eBNAME_0].center);
    box_draw(&field[eBNAME_0].point, &field[eBNAME_0].form, 0);
}

void print_field(WINDOW *win)
{
    int i;

    for (i = eBNAME_0; i < eBNAME_MAX; i++)
    {
        if (field[i].highlight)
        {
            attron(COLOR_PAIR(1));
            mvprintw(field[i].center.y, field[i].center.x, field[i].name);
            attroff(COLOR_PAIR(1));
        }
        else
            mvprintw(field[i].center.y, field[i].center.x, field[i].name);
    }
}

void init_field(void)
{
    init_pads();

}









int startx = 0;
int starty = 0;

void proc_field_click(int mouse_x, int mouse_y);

int main()
{   int c;
    WINDOW *menu_win;
    MEVENT event;

    /* Initialize curses */
    initscr();
    clear();
    noecho();
    cbreak();   //Line buffering disabled. pass on everything

    start_color();
    init_pair(1, COLOR_WHITE, COLOR_RED);



    attron(A_REVERSE);
    mvprintw(23, 1, "Click on Exit to quit (Works best in a virtual console)");
    refresh();
    attroff(A_REVERSE);

    /* Print the menu for the first time */
    menu_win = newwin(HEIGHT, WIDTH, starty, startx);
    keypad(menu_win, TRUE);
    /* Get all the mouse events */
    mousemask(ALL_MOUSE_EVENTS, NULL);
    
    box(menu_win, 0, 0);
    wrefresh(menu_win);

    init_field();
    print_field(menu_win);
    refresh();

    while(1)
    {
        c = wgetch(menu_win);
        switch(c)
        {
            case KEY_MOUSE:
                if(getmouse(&event) == OK)
                {
                    /* When the user clicks left mouse button */
                    if(event.bstate & BUTTON1_CLICKED)
                    {   
                        proc_field_click(event.x, event.y);
                        print_field(menu_win);
                    }
                }
            break;
        }

        refresh(); 
    }
end:
    endwin();
    return 0;
}

/* Report the choice according to mouse position */
void proc_field_click(int mouse_x, int mouse_y)
{   
    int i;

    for (i = 0; i < eBNAME_MAX; i++)
    {
        if (((mouse_x >= field[i].point.x) && (field[i].point.x + field[i].form.width) > mouse_x) &&
            ((mouse_y >= field[i].point.y) && (field[i].point.y + field[i].form.height) > mouse_y))
        {
            field[i].highlight = 1;
        }
        else
        {
            field[i].highlight = 0;
        }
    }
}
