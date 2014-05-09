#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>

#define KEYPAD_FILE "/dev/10_button_keypad"

int fd;

void correctExit(int sig)
{
	printf("\nexit normal\n");
	close(fd);
	exit(0);
}

int main(void)
{
	fd_set rfds;
	int selectRetVal;
	char newSymbol;

	fd = open(KEYPAD_FILE, O_RDONLY);
	if(fd < 0){
		printf("Open: Cannot open file.\n");
		return 1;
	}

	signal(SIGINT, correctExit);
	
	FD_ZERO(&rfds);
	FD_SET(fd,&rfds);

	while(1){
		selectRetVal = select(fd + 1, &rfds, NULL, NULL, NULL);
		if(selectRetVal == -1){
			printf("\nSelect: Select error.\n");
			close(fd);
			return 1;
		}
		if(FD_ISSET(fd, &rfds)){
			read(fd, &newSymbol, 1);
			printf("-> %c (%d)\n", newSymbol, newSymbol);
		}
	}
return 0;
}
