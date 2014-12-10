#include <stdio.h>

int main(int argc, char *argv[])
{
	char command[256] = {0};
	
	sprintf (command, "echo \"Hello from Node <%s> on $PWD\"", argv[1]);
	system (command);

	return 0;
}