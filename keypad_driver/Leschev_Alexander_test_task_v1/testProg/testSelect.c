#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>

#define KEYPAD_FILE "/dev/10_button_keypad"

int fd;

void correct_exit(int sig)
{
	printf("\nexit normal\n");
	close(fd);
	exit(0);
}

int main(void)
{
	fd_set rfds;
	int select_ret_val;
	char new_symbol;

	fd = open(KEYPAD_FILE, O_RDONLY);
	if(fd < 0){
		printf("Open: Cannot open file.\n");
		return 1;
	}

	signal(SIGINT, correct_exit);
	
	FD_ZERO(&rfds);
	FD_SET(fd,&rfds);

	while(1){
		select_ret_val = select(fd + 1, &rfds, NULL, NULL, NULL);
		if(select_ret_val == -1){
			printf("\nSelect: Select error.\n");
			close(fd);
			return 1;
		}
		if(FD_ISSET(fd, &rfds)){
			read(fd, &new_symbol, 1);
			printf("-> %c (%d)\n", new_symbol, new_symbol);
		}
	}
return 0;
}
