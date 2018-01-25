#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define trace(arg, ...) printf(arg"\n", ##__VA_ARGS__)

int VOLUME_MIN = 1;
int VOLUME_MAX = 100;
int VOLUME_DEF = 50;

void proc_volume_changes(char *display, int width, int vol)
{
	int i, offset = 0, coeficient;
	int max_arrow, cur_arrow, empty_arrow;

	offset += sprintf(&display[offset], "sprk[");
	coeficient = (width - offset - 1/*]*/);

	if (VOLUME_MAX > coeficient)
	{
		max_arrow = (int)(((double)VOLUME_MAX/(double)coeficient) + 0.5/*okryglyaem*/); /*koeficient - skolko vol v odnom delenii*/
		cur_arrow = vol / max_arrow; /* tekyshee kol-vo delenii */
		
		/* otrzayem lishnee symboli cotorie bolshe maximalno vozmoznogo znachenia */
		empty_arrow = (int)((((double)VOLUME_MAX / (double)max_arrow) + 0.5/*okryglyaem*/)) - cur_arrow;
	}
	else
	{
		max_arrow = (int)(((double)coeficient/(double)VOLUME_MAX) + 0.5/*okryglyaem*/);
		cur_arrow = vol * max_arrow; /* tekyshee kol-vo delenii */

		/* otrzayem lishnee symboli cotorie bolshe maximalno vozmoznogo znachenia */
		empty_arrow = (VOLUME_MAX * max_arrow) - cur_arrow;
	}

	for (i = 0; i < cur_arrow; i++)
		display[offset++] = '>';

	for (i = 0; i < empty_arrow; i++)
		display[offset++] = '_';

	display[offset++] = ']';

	printf("%s [%03d/%03d] ", display, vol, VOLUME_MAX);
}

int main(int argc, char *argv[])
{
	int width = 16;
	char input;
	char *display_line;

	trace("help: %s <display width> <max volume> <min volume> <def volume>", argv[0]);

	if (argc >= 2)
	{
		width = atoi(argv[1]);
		if (width > 40)
			width = 40;
		else if (width < 16)
			width = 16;

		trace("change default width to %d", width);
	}
	if (argc >= 3)
	{
		VOLUME_MAX = atoi(argv[2]);
		if (VOLUME_MAX < 2 || VOLUME_MAX > 100)
			VOLUME_MAX = 100;

		trace("change default volume max to %d", VOLUME_MAX);

	}
	if (argc >= 4)
	{
		VOLUME_MIN = atoi(argv[3]);
		if (VOLUME_MIN < 0 || VOLUME_MIN >= VOLUME_MAX)
			VOLUME_MIN = 1;

		trace("change default volume min to %d", VOLUME_MIN);

	}
	if (argc >= 5)
	{
		VOLUME_DEF = atoi(argv[4]);
		if (VOLUME_DEF < VOLUME_MIN || VOLUME_DEF > VOLUME_MAX)
			VOLUME_DEF = 50;

		trace("change default volume def to %d", VOLUME_DEF);
	}

	display_line = (char *)calloc(1, width);

	proc_volume_changes(display_line, width, VOLUME_DEF);
	trace("w - up; s - down");

	while((input = getchar()) != 'q')
	{
		switch(input)
		{
			case 'w':
				if ((++VOLUME_DEF) > VOLUME_MAX)
					VOLUME_DEF = VOLUME_MAX;
				break;

			case 's':
				if ((--VOLUME_DEF) < VOLUME_MIN)
					VOLUME_DEF = VOLUME_MIN;
				break;

			default:
				continue;
			
		}

		proc_volume_changes(display_line, width, VOLUME_DEF);
		trace("w - up; s - down");
	}

	return 0;
}