#include <ncurses.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#define NUMPAD_FIELD_X 0
#define NUMPAD_FIELD_Y 0

#define NUMPAD_WIDTH 50
#define NUMPAD_HEIGHT 20

#define BUTTON_WIDTH 4
#define BUTTON_HEIGHT 2

#define LOG_WINDOW_X NUMPAD_WIDTH + 1
#define LOG_WINDOW_Y 1
#define LOG_WIDTH 100
#define LOG_HEIGHT 20

#define LCD_HEIGHT 2
#define LCD_WIDTH 16

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

    eBCODE_EXT_0,
    eBCODE_EXT_1,
    eBCODE_EXT_2,
    eBCODE_EXT_3,
    eBCODE_EXT_4,
    eBCODE_EXT_5,
    eBCODE_EXT_6,
    eBCODE_EXT_7,
    eBCODE_EXT_8,

    eBCODE_LCD,

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
    char text[32];
    int text_len;

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
    point_t point;
    form_t form;

    btn_t numpad[eBCODE_MAX];
    btn_t lcd;
    // log_t log;
} field_t;

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
        memcpy(point, &box->point, sizeof(point_t));
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

void btn_numbers_init(btn_t *numpad)
{
    int i;
    int x_border_offset = 1, y_border_offset = 1;
    int x = NUMPAD_FIELD_X + x_border_offset, y = NUMPAD_FIELD_Y + y_border_offset;

    for (i = eBCODE_0; i <= eBCODE_HASH; i++)
    {
        if (i == eBCODE_STAR || i == eBCODE_HASH || i == eBCODE_0)
        {
            numpad[i].form.height = BUTTON_HEIGHT;
            numpad[i].form.width = BUTTON_WIDTH;
            numpad[i].text_len = 1;
            continue;
        }

        // numpad[i].highlight = 0;
        numpad[i].point.x = x;
        numpad[i].point.y = y;
        numpad[i].form.height = BUTTON_HEIGHT;
        numpad[i].form.width = BUTTON_WIDTH;
        numpad[i].code = i;
        numpad[i].text[0] = i + 0x30;
        numpad[i].text_len = 1;

        btn_get_center(&numpad[i], numpad[i].text_len, &numpad[i].center);

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
    numpad[eBCODE_STAR].text[0] = '*';
    btn_get_center(&numpad[eBCODE_STAR], numpad[eBCODE_STAR].text_len, &numpad[eBCODE_STAR].center);
    btn_draw(&numpad[eBCODE_STAR].point, &numpad[eBCODE_STAR].form, 0);

    numpad[eBCODE_0].point.x = x_border_offset + BUTTON_WIDTH;
    numpad[eBCODE_0].point.y = y_border_offset + BUTTON_HEIGHT * 3;
    numpad[eBCODE_0].text[0] = '0';
    btn_get_center(&numpad[eBCODE_0], numpad[eBCODE_0].text_len, &numpad[eBCODE_0].center);
    btn_draw(&numpad[eBCODE_0].point, &numpad[eBCODE_0].form, 0);

    numpad[eBCODE_HASH].point.x = x_border_offset + BUTTON_WIDTH * 2;
    numpad[eBCODE_HASH].point.y = y_border_offset + BUTTON_HEIGHT * 3;
    numpad[eBCODE_HASH].text[0] = '#';
    btn_get_center(&numpad[eBCODE_HASH], numpad[eBCODE_HASH].text_len, &numpad[eBCODE_HASH].center);
    btn_draw(&numpad[eBCODE_HASH].point, &numpad[eBCODE_HASH].form, 0);
}

void btn_sys_init(btn_t *numpad)
{
    int i;
    int x  = NUMPAD_FIELD_X + (BUTTON_WIDTH * 4);
    int y = NUMPAD_FIELD_Y + 1;
    char *text[] = {"HOLD", "XPER", "LINE", "SPKR"};

    for (i = eBCODE_HOLD; i <= eBCODE_SPKR; i++)
    {
        numpad[i].point.x = x;
        numpad[i].point.y = y;
        numpad[i].form.height = BUTTON_HEIGHT;
        numpad[i].form.width = BUTTON_WIDTH;
        numpad[i].code = i;
        numpad[i].text_len = sprintf(numpad[i].text, "%s", text[i - eBCODE_HOLD]);
        btn_get_center(&numpad[i], numpad[i].text_len, &numpad[i].center);
        btn_draw(&numpad[i].point, &numpad[i].form, 0);

        y += BUTTON_HEIGHT;
    }
}

void btn_updown_init(btn_t *numpad)
{
    int i;
    int x  = NUMPAD_FIELD_X + 1;
    int y = NUMPAD_FIELD_Y + (BUTTON_HEIGHT * 5);
    char *text[] = {"-", "+"};

    for (i = eBCODE_PLUS; i <= eBCODE_MINUS; i++)
    {
        numpad[i].point.x = x;
        numpad[i].point.y = y;
        numpad[i].form.height = BUTTON_HEIGHT;
        numpad[i].form.width = BUTTON_WIDTH;
        numpad[i].code = i;
        numpad[i].text_len = sprintf(numpad[i].text, "%s", text[i - eBCODE_PLUS]);
        btn_get_center(&numpad[i], numpad[i].text_len, &numpad[i].center);
        btn_draw(&numpad[i].point, &numpad[i].form, 0);

        x += BUTTON_WIDTH;
    }
}

void btn_ext_init(btn_t *numpad)
{
    int i;
    int x  = NUMPAD_FIELD_X + (BUTTON_WIDTH * 6);
    int y = NUMPAD_FIELD_Y + 1;
    char *text[] = {"ext0", "ext1", "ext2", "ext3", "ext4",
                    "ext5", "ext6", "ext7", "ext8"};

    for (i = eBCODE_EXT_0; i <= eBCODE_EXT_8; i++)
    {
        numpad[i].point.x = x;
        numpad[i].point.y = y;
        numpad[i].form.height = BUTTON_HEIGHT;
        numpad[i].form.width = BUTTON_WIDTH;
        numpad[i].code = i;
        numpad[i].text_len = sprintf(numpad[i].text, "%s", text[i - eBCODE_EXT_0]);
        btn_get_center(&numpad[i], numpad[i].text_len, &numpad[i].center);
        btn_draw(&numpad[i].point, &numpad[i].form, 0);

        y += BUTTON_HEIGHT;
    }
}

int lcd_clear(btn_t *lcd, int line_num)
{
    int i;

    if (line_num >= LCD_HEIGHT)
        return -1;

    for (i = 0; i < LCD_WIDTH; i++)
        mvprintw(lcd->point.y + line_num + 1, lcd->point.x + i + 1, " ");
}

int lcd_print(btn_t *lcd, int line_num, char *text, int len)
{
    if (line_num >= LCD_HEIGHT)
        return -1;

    lcd_clear(lcd, line_num);

    lcd->text_len = sizeof(lcd->text) / LCD_HEIGHT;
    strncpy(lcd->text, text, lcd->text_len);
    mvprintw(lcd->point.y + line_num + 1, lcd->point.x + 1, "%.*s", lcd->text_len, lcd->text);
}

void lcd_init(btn_t *lcd)
{
    int i;
    int x  = NUMPAD_FIELD_X + (BUTTON_WIDTH * 8);
    int y = NUMPAD_FIELD_Y + 1;

    memset(lcd, 0, sizeof(*lcd));

    lcd->point.x = x;
    lcd->point.y = y;
    lcd->form.height = LCD_HEIGHT + 1;
    lcd->form.width = LCD_WIDTH + 1;
    lcd->code = eBCODE_LCD;
    btn_draw(&lcd->point, &lcd->form, 0);

    mvprintw(lcd->point.y, lcd->point.x + LCD_WIDTH/2, "LCD");
}

void field_init(field_t *field)
{
    memset(field->numpad, 0, sizeof(field->numpad));

    btn_numbers_init(field->numpad);
    btn_sys_init(field->numpad);
    btn_updown_init(field->numpad);
    btn_ext_init(field->numpad);
    lcd_init(&field->lcd);

    field->point.x = NUMPAD_FIELD_X;
    field->point.y = NUMPAD_FIELD_Y;
    field->form.height= NUMPAD_HEIGHT;
    field->form.width = NUMPAD_WIDTH;

    btn_draw(&field->point, &field->form, 0);
}

void field_print(field_t *field)
{
    int i;
    btn_t *numpad = field->numpad;

    for (i = eBCODE_0; i < eBCODE_MAX; i++)
    {
        if (numpad[i].highlight)
        {
            attron(COLOR_PAIR(1));
            mvprintw(numpad[i].center.y, numpad[i].center.x, "%.*s", numpad[i].text_len, numpad[i].text);
            attroff(COLOR_PAIR(1));
        }
        else
            mvprintw(numpad[i].center.y, numpad[i].center.x, "%.*s", numpad[i].text_len, numpad[i].text);
    }
}

int proc_numpad_click(btn_t *numpad, int mouse_x, int mouse_y)
{
    int i;
    int num_code = -1;

    for (i = 0; i < eBCODE_MAX; i++)
    {
        if (i == eBCODE_LCD)
            continue;

        if (((mouse_x >= numpad[i].point.x) && (numpad[i].point.x + numpad[i].form.width) > mouse_x) &&
            ((mouse_y >= numpad[i].point.y) && (numpad[i].point.y + numpad[i].form.height) > mouse_y))
        {
            numpad[i].highlight = 1;
            num_code = i;
            numlog(&log, "Pressed '%.*s'", numpad[i].text_len, numpad[i].text);
        }
        else
        {
            numpad[i].highlight = 0;
        }
    }

    return num_code;
}

int main()
{
    int c;
    int startx = 0, starty = 0;
    WINDOW *menu_win;
    MEVENT event;
    field_t field;
    int prev_b = -1, cur_b = -1;

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

    // box(menu_win, 0, 0);
    wrefresh(menu_win);

    log_init(&log);

    field_init(&field);
    field_print(&field);

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
                        if ((cur_b = proc_numpad_click(field.numpad, event.x, event.y)) > 0)
                        {
                            char logstr[64] = {0};

                            sprintf(logstr, "Cur:  %s", (cur_b >= 0)? field.numpad[cur_b].text:"none");
                            lcd_print(&field.lcd, 1, logstr, strlen(logstr));

                            sprintf(logstr, "Prev: %s", (prev_b >= 0)? field.numpad[prev_b].text:"none");
                            lcd_print(&field.lcd, 0, logstr, strlen(logstr));

                            prev_b = cur_b;
                        }

                        field_print(&field);
                    }
                }
            break;

            default:
            {
                numlog(&log, "Pressed key %d (%s)", c, keyname(c));
            }
        }

        refresh();
    }

end:
    endwin();
    return 0;
}
