#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <poll.h>

#define KEYPAD_FILE "/dev/10_button_keypad"
#define TIMEOUT 100

int fd;

void correct_exit(int sig)
{
	printf("\nexit normal\n");
	close(fd);
	exit(0);
}

int main(void)
{
	struct pollfd fds[1];
	int poll_ret_val;
	char new_symbol;

	fd = open(KEYPAD_FILE, O_RDONLY);
	if(fd < 0){
		printf("Open: Cannot open file.\n");
		return 1;
	}

	signal(SIGINT, correct_exit);

	fds[0].fd = fd;
	fds[0].events = POLLIN;
	
	while(1){
		poll_ret_val = poll(fds, 1, TIMEOUT);

		if (poll_ret_val == -1){
			printf("Poll: Poll error.\n");
			return 1;
		}
		if (fds[0].revents & POLLIN){
			read(fd, &new_symbol, 1);
			printf("-> %c (%d)\n", new_symbol, new_symbol);
		}
	}
return 0;
}
