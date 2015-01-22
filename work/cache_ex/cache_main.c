#include <stdio.h>

#define MAX_NET_IFACE 400
#define MAX_SETS 40
#define MAX_SET_LINES (MAX_NET_IFACE / MAX_SETS)
#define SETS_IDX(x) (x % MAX_SETS)
#define IDX_INVALID -1

#define xfree(x) do { if (x) free(x); x = NULL; } while(0)

typedef struct {
    int data;
} data_t;

typedef struct {
    int ID;
    int busy;
    data_t data;
} cache_line_t

typedef struct {
    cache_line_t *lines;
} set_t;

set_t *sets = NULL;

void cache_sets_init()
{
    int i;

    if (sets)
        cache_sets_deinit();

    sets = (set_t *)calloc(1, sizeof (set_t) * MAX_SETS);
    if (!sets)
    {
        /* err */
        return;
    }

    for (i = 0; i < MAX_SETS; i++)
    {
        sets[i].lines = (cache_line_t *)calloc(1, sizeof(cache_line_t) * MAX_SET_LINES);
        if (!sets->lines)
        {
            /* err */
            cache_sets_deinit();
            return;
        }
    }
}

void cache_sets_deinit()
{
    int i;

    if (!sets)
        return;

    for (i = 0; i < MAX_SETS; i++)
        xfree (sets[i].lines);

    xfree(sets);
}

void cache_sets_add(int ID, data_t *data)
{
    int i;
    int set_of = 0;
    cache_line_t *p_cache_line;

    if (!sets || !data)
        return;

    set_of = SETS_IDX(ID);

    if (!sets[set_of].lines)
        return;

    p_cache_line = sets[set_of].lines;
    if (!p_cache_line)
        return;

    for (i = 0; i < MAX_SET_LINES; i++)
    {
        if (p_cache_line[i].ID == ID)
        {
            memcpy(&p_cache_line[i].data, data, sizeof (data_t));
            return;
        }
    }

    for (i = 0; i < MAX_SET_LINES; i++)
    {
        if (p_cache_line[i].busy)
            continue;

        memcpy(&p_cache_line[i].data, data, sizeof (data_t));
        p_cache_line[i].busy = 1;
        return;
    }
}

void cache_sets_delete(int ID)
{
    int i;
    int set_of = 0;
    cache_line_t *p_cache_line;

    set_of = SETS_IDX(ID);
    p_cache_line = sets[set_of].lines;

    for (i = 0; i < MAX_SET_LINES; i++)
    {
        if (!p_cache_line[i].busy)
            continue;

        if (p_cache_line[i].ID != ID)
            continue;

        memcpy(&p_cache_line[i].data, 0, sizeof (data_t));
        return;
    }
}

int cache_sets_get(int ID, data_t *data)
{
    int i;
    int set_of = 0;
    cache_line_t *p_cache_line;

    if (!data)
        return 1;

    set_of = SETS_IDX(ID);
    p_cache_line = sets[set_of].lines;

    memset(data, 0, sizeof(data_t));

    for (i = 0; i < MAX_SET_LINES; i++)
    {
        if (!p_cache_line[i].busy)
            continue;

        if (p_cache_line[i].ID != ID)
            continue;

        memcpy(data, &p_cache_line[i].data, sizeof (data_t));
        return;
    }
}

int main(void)
{

    return 0;
}