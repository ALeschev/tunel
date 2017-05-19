#include <ncurses.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#define NUMPAD_FIELD_X 0
#define NUMPAD_FIELD_Y 0

#define NUMPAD_WIDTH 100
#define NUMPAD_HEIGHT 20

#define BUTTON_WIDTH 4
#define BUTTON_HEIGHT 2

#define LOG_WINDOW_X NUMPAD_WIDTH + 1
#define LOG_WINDOW_Y 1
#define LOG_WIDTH 100
#define LOG_HEIGHT 60

typedef enum
{
    eBCODE_0 = 0,
    eBCODE_1,
    eBCODE_2,
    eBCODE_3,
    eBCODE_4,
    eBCODE_5,
    eBCODE_6,
    eBCODE_7,
    eBCODE_8,
    eBCODE_9,
    eBCODE_STAR,
    eBCODE_HASH,

    eBCODE_PLUS,
    eBCODE_MINUS,

    eBCODE_HOLD,
    eBCODE_XPER,
    eBCODE_LINE,
    eBCODE_SPKR,

    eBCODE_MAX
} btn_code_t;

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

    btn_code_t code;

    point_t point;
    point_t center;
    form_t form;

    int highlight;
} btn_t;

typedef struct
{
    char area[LOG_HEIGHT][LOG_WIDTH];
    char len[LOG_HEIGHT];
    int position;
} log_t;

typedef struct
{
    btn_t numpad[eBCODE_MAX];
    // log_t log;
} filed_t;

static log_t log;

void log_init(log_t *log)
{
    memset(log, 0, sizeof(log_t));
    log->position = LOG_WINDOW_Y;
}

void log_rotate(log_t *log)
{
    int i;

    for (i = 0; i < LOG_HEIGHT-1; i++)
    {
        memcpy(log->area[i], log->area[i+1], strlen(log->area[i+1]));
        log->len[i] = log->len[i+1];
        move(LOG_WINDOW_Y + i, LOG_WINDOW_X);
        clrtoeol();
        mvprintw(LOG_WINDOW_Y + i, LOG_WINDOW_X, "%.*s", log->len[i], log->area[i]);
    }

    log->position = LOG_HEIGHT-1;

    move(LOG_HEIGHT, LOG_WINDOW_X);
    clrtoeol();
}

void log_clear(log_t *log)
{
    int y;

    // memset(log->area, 0, LOG_HEIGHT * LOG_WIDTH);
    // log->position = LOG_WINDOW_Y;

    log_init(log);

    for (y = LOG_WINDOW_Y; y <= LOG_HEIGHT; y++)
    {
        move(y, LOG_WINDOW_X);
        clrtoeol();
    }
}

void numlog(log_t *log, char *format, ...)
{
    va_list ap;
    int length, size;
    char str[4096], *p;
    struct timeval tp;
    struct tm *pt;
    time_t t;
    static int i = 0;

    if (log->position >= LOG_HEIGHT)
        log_rotate(log);

    gettimeofday(&tp, NULL);
    t = (time_t)tp.tv_sec;
    pt = localtime(&t);
    length = sprintf(str, "%02d:%02d:%02d.%06lu ", pt->tm_hour, pt->tm_min, pt->tm_sec, tp.tv_usec);

    size = sizeof(str) - length - 10;

    va_start(ap, format);
    length += vsnprintf(&str[length], size, format, ap);
    va_end(ap);

    size = strlen(str);
    while(size && (str[size-1] == '\n'))
        size--;

    if (length >= LOG_WIDTH)
    {
        log->len[log->position] = LOG_WIDTH-1;
        strncpy(log->area[log->position], str, LOG_WIDTH-1);
        mvprintw(log->position, LOG_WINDOW_X, log->area[log->position]);
        log->position++;
        numlog(log, "%s", &str[LOG_WIDTH-1]);
    }
    else
    {
        strncpy(log->area[log->position], str, length);
        log->len[log->position] = length;
        mvprintw(log->position, LOG_WINDOW_X, "%.*s", length, log->area[log->position]);
        log->position++;
    }

    if (log->position >= LOG_HEIGHT)
        log_rotate(log);
}

void btn_get_center(btn_t *box, int content_len, point_t *point)
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

void btn_draw(point_t *point, form_t *form, bool clear)
{   
    int i, j;

    if(!clear)
    {
        mvaddch(point->y, point->x, ACS_ULCORNER);
        mvaddch(point->y, point->x + form->width, ACS_URCORNER);
        mvaddch(point->y + form->height, point->x, ACS_LLCORNER);
        mvaddch(point->y + form->height, point->x + form->width, ACS_LRCORNER);
        mvhline(point->y, point->x + 1, ACS_HLINE, form->width - 1);
        mvhline(point->y + form->height, point->x + 1, ACS_HLINE, form->width - 1);
        mvvline(point->y + 1, point->x, ACS_VLINE, form->height - 1);
        mvvline(point->y + 1, point->x + form->width, ACS_VLINE, form->height - 1);
    }
    else
    {
        for(j = point->y; j <= point->y + form->height; ++j)
            for(i = point->x; i <= point->x + form->width; ++i)
                mvaddch(j, i, ' ');
    }
}

void btn_init(btn_t *numpad)
{
    int i;
    int x_border_offset = 1, y_border_offset = 1;
    int x = NUMPAD_FIELD_X + 1, y = NUMPAD_FIELD_Y + 1;

    memset(numpad, 0, sizeof(*numpad) * eBCODE_MAX);

    for (i = eBCODE_0; i <= eBCODE_HASH; i++)
    {
        if (i == eBCODE_STAR || i == eBCODE_HASH || i == eBCODE_0)
        {
            numpad[i].form.height = BUTTON_HEIGHT;
            numpad[i].form.width = BUTTON_WIDTH;
            numpad[i].name_len = 1;
            continue;
        }

        // numpad[i].highlight = 0;
        numpad[i].point.x = x;
        numpad[i].point.y = y;
        numpad[i].form.height = BUTTON_HEIGHT;
        numpad[i].form.width = BUTTON_WIDTH;
        numpad[i].code = i;
        numpad[i].name[0] = i + 0x30;
        numpad[i].name_len = 1;

        btn_get_center(&numpad[i], numpad[i].name_len, &numpad[i].center);

        if (!(i % 3))
        {
            y += BUTTON_HEIGHT;
            x = 1;
        } 
        else
        {
            x += BUTTON_WIDTH;
        }

        btn_draw(&numpad[i].point, &numpad[i].form, 0);
    }

    numpad[eBCODE_STAR].point.x = x_border_offset;
    numpad[eBCODE_STAR].point.y = y_border_offset + BUTTON_HEIGHT * 3;
    numpad[eBCODE_STAR].name[0] = '*';
    btn_get_center(&numpad[eBCODE_STAR], numpad[eBCODE_STAR].name_len, &numpad[eBCODE_STAR].center);
    btn_draw(&numpad[eBCODE_STAR].point, &numpad[eBCODE_STAR].form, 0);

    numpad[eBCODE_HASH].point.x = x_border_offset + BUTTON_WIDTH * 2;
    numpad[eBCODE_HASH].point.y = y_border_offset + BUTTON_HEIGHT * 3;
    numpad[eBCODE_HASH].name[0] = '#';
    btn_get_center(&numpad[eBCODE_HASH], numpad[eBCODE_HASH].name_len, &numpad[eBCODE_HASH].center);
    btn_draw(&numpad[eBCODE_HASH].point, &numpad[eBCODE_HASH].form, 0);

    numpad[eBCODE_0].point.x = x_border_offset + BUTTON_WIDTH;
    numpad[eBCODE_0].point.y = y_border_offset + BUTTON_HEIGHT * 3;
    numpad[eBCODE_0].name[0] = '0';
    btn_get_center(&numpad[eBCODE_0], numpad[eBCODE_0].name_len, &numpad[eBCODE_0].center);
    btn_draw(&numpad[eBCODE_0].point, &numpad[eBCODE_0].form, 0);
}

void numpad_print(btn_t *numpad)
{
    int i;

    for (i = eBCODE_0; i < eBCODE_MAX; i++)
    {
        if (numpad[i].highlight)
        {
            attron(COLOR_PAIR(1));
            mvprintw(numpad[i].center.y, numpad[i].center.x, "%.*s", numpad[i].name_len, numpad[i].name);
            attroff(COLOR_PAIR(1));
        }
        else
            mvprintw(numpad[i].center.y, numpad[i].center.x, "%.*s", numpad[i].name_len, numpad[i].name);
    }
}

void numpad_init(btn_t *numpad)
{
    btn_init(numpad);

}

void proc_numpad_click(btn_t *numpad, int mouse_x, int mouse_y)
{
    int i;

    for (i = 0; i < eBCODE_MAX; i++)
    {
        if (((mouse_x >= numpad[i].point.x) && (numpad[i].point.x + numpad[i].form.width) > mouse_x) &&
            ((mouse_y >= numpad[i].point.y) && (numpad[i].point.y + numpad[i].form.height) > mouse_y))
        {
            numpad[i].highlight = 1;
            numlog(&log, "Pressed '%.*s'", numpad[i].name_len, numpad[i].name);
        }
        else
        {
            numpad[i].highlight = 0;
        }
    }
}

int main()
{
    int c;
    int startx = 0, starty = 0;
    WINDOW *menu_win;
    MEVENT event;
    filed_t filed;

    /* Initialize curses */
    initscr();
    clear();
    noecho();
    cbreak();   //Line buffering disabled. pass on everything

    start_color();
    init_pair(1, COLOR_WHITE, COLOR_RED);

    menu_win = newwin(NUMPAD_HEIGHT, NUMPAD_WIDTH, NUMPAD_FIELD_Y, NUMPAD_FIELD_X);
    keypad(menu_win, TRUE);

    mousemask(ALL_MOUSE_EVENTS, NULL);

    box(menu_win, 0, 0);
    wrefresh(menu_win);

    log_init(&log);

    numpad_init(filed.numpad);
    numpad_print(filed.numpad);

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
                        proc_numpad_click(filed.numpad, event.x, event.y);
                        numpad_print(filed.numpad);
                    }
                }
            break;

            default:
                numlog(&log, "Pressed key %d (%s)", c, keyname(c));
        }

        refresh(); 
    }

end:
    endwin();
    return 0;
}
