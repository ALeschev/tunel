#include "src.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) 
{
	Field *curField = new Field();
	Print *printThisShit = new Print();

	// while (true) {
	// 	if (curField->Configure() == 1)
	// 		break;
	// 	printThisShit->show(curField);
	// }

	while (true) {
		system("clear");
		printThisShit->show(curField);
		curField->Generation();
		getchar();
	}

	return 0;
}