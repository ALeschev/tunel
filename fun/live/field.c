#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define FIELD_WIDTH 20 /* ширина */
#define FIELD_HEIGHT 10 /* высота */

int field[FIELD_HEIGHT][FIELD_WIDTH];
int neighbours[FIELD_HEIGHT][FIELD_WIDTH];

int get_neighbours(int y, int x)
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
        if (field[y-1][x-1])
            neighbours++;

        /**
         * #|*|#
         * #|@|#
         * #|#|#
         */
        if (field[y-1][x])
            neighbours++;

        /**
         * #|#|*
         * #|@|#
         * #|#|#
         */
        if (field[y-1][x+1])
            neighbours++;

        /**
         * #|#|#
         * *|@|#
         * #|#|#
         */
        if (field[y][x-1])
            neighbours++;

        /**
         * #|#|#
         * #|@|*
         * #|#|#
         */
        if (field[y][x+1])
            neighbours++;

        /**
         * #|#|#
         * #|@|#
         * *|#|#
         */
        if (field[y+1][x-1])
            neighbours++;

        /**
         * #|#|#
         * #|@|#
         * #|*|#
         */
        if (field[y+1][x])
            neighbours++;

        /**
         * #|#|#
         * #|@|#
         * #|#|*
         */
        if (field[y+1][x+1])
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
            if (field[y][x-1])
                neighbours++;

            if (field[y+1][x-1])
                neighbours++;
        }

        if (x != FIELD_WIDTH-1)
        {
            if (field[y][x+1])
                neighbours++;

            if (field[y+1][x+1])
                neighbours++;
        }

        if (field[y+1][x])
            neighbours++;
    } else
    if (!x)
    {
        /**
         * #|#
         * @|#
         * #|#
         */

        if (field[y-1][x])
            neighbours++;

        if (field[y-1][x+1])
            neighbours++;

        if (field[y][x+1])
            neighbours++;

        if (y != FIELD_HEIGHT-1)
        {
            if (field[y+1][x])
                neighbours++;

            if (field[y+1][x+1])
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

        if (field[y-1][x])
            neighbours++;

        if (field[y-1][x-1])
            neighbours++;

        if (field[y][x-1])
            neighbours++;

        if (y != FIELD_HEIGHT-1)
        {
            if (field[y+1][x-1])
                neighbours++;

            if (field[y+1][x])
                neighbours++;
        }
    } else
    if (y == FIELD_HEIGHT-1)
    {
        /**
         * #|#|#
         * #|@|#
         */

        if (field[y-1][x-1])
            neighbours++;

        if (field[y-1][x])
            neighbours++;

        if (field[y-1][x+1])
            neighbours++;

        if (field[y][x-1])
            neighbours++;

        if (field[y][x+1])
            neighbours++;
    }

    return neighbours;
}

void field_print(int field[FIELD_HEIGHT][FIELD_WIDTH])
{
    int x, y;

    printf("------------\n");

    for (y = 0; y < FIELD_HEIGHT; y++)
    {
        for (x = 0; x < FIELD_WIDTH; x++)
        {
            printf("%d ", field[y][x]);
        }

        printf("\n");
    }
}

int main(void)
{
    int x, y;

    srand(time(NULL));

    for (y = 0; y < FIELD_HEIGHT; y++)
        for (x = 0; x < FIELD_WIDTH; x++)
            field[y][x] = rand()%2;

    field_print(field);

    for (y = 0; y < FIELD_HEIGHT; y++)
    {
        for (x = 0; x < FIELD_WIDTH; x++)
        {
            neighbours[y][x] = get_neighbours(y, x);
        }
    }

    field_print(neighbours);

    return 0;
}