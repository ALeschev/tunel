#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

#include "frame.h"

unsigned long sys_tick_count_msec(void)
{
	struct timespec tp;

	clock_gettime(CLOCK_MONOTONIC, &tp);

	return (tp.tv_sec*1000 + tp.tv_nsec/1000000);
}

int connection_handler(int connection_fd)
{
	int	i, res, offset = 0;
	char buffer[MAX_BUF];
	message_t *message;
	unsigned long start_time, cur_time;
	unsigned long bytes = 0;

	message = (message_t *)buffer;
	message->header.plen = MAX_BUF - sizeof(message_t);
	memset(message->payload, 0xFF, message->header.plen);

	cur_time = start_time = sys_tick_count_msec();

	while(cur_time - start_time < TEST_TIME)
	{
		for (i = 0; i < MSG_PER_CYCLE; i++)
		{
			message->header.msg_num++;
			offset = 0;
			// printf("send msg %lu plen %u\n", message->header.msg_num, message->header.plen);
			do
			{
				res = write(connection_fd, &buffer[offset], MAX_BUF - offset);
				if (res < 0)
				{
					if (errno == EINTR)
					{
						printf("catch write EINTR: %s\n", strerror(errno));
						continue;
					}
					if (errno == EAGAIN)
					{
						printf("catch write EAGAIN: %s\n", strerror(errno));
						continue;
					}

					printf("catch write error: %s\n", strerror(errno));

					goto exit__;
				}
				offset += res;
			} while (offset < MAX_BUF && res > 0);

			bytes += offset;
		}

		usleep(DELAY_PER_CYCLE);

		cur_time = sys_tick_count_msec();
	}

	message->header.msg_num++;
	message->header.cmd = CMD_DIE;
	message->header.plen = 0;

	bytes += write(connection_fd, buffer, sizeof(message->header));

exit__:
	printf("send %lu messages %lu bytes\n", message->header.msg_num, bytes);

	close(connection_fd);
	return 0;
}

int main(void)
{
	struct sockaddr_un address;
	int socket_fd, connection_fd;
	socklen_t address_length;
	pid_t child;

	socket_fd = socket(PF_LOCAL, SOCK_STREAM, 0);
	if(socket_fd < 0)
	{
	printf("socket() failed\n");
	return 1;
	}

	unlink("./demo_socket");

	memset(&address, 0, sizeof(struct sockaddr_un));

	address.sun_family = AF_LOCAL;
	snprintf(address.sun_path, sizeof(address.sun_path), "./demo_socket");

	if(bind(socket_fd, (struct sockaddr *) &address, sizeof(struct sockaddr_un)) != 0)
	{
		printf("bind() failed\n");
		return 1;
	}

	if(listen(socket_fd, 5) != 0)
	{
		printf("listen() failed\n");
		return 1;
	}

	address_length = sizeof(address);
	while((connection_fd = accept(socket_fd, (struct sockaddr *) &address, &address_length)) > -1)
	{
		child = fork();
		if(child == 0)
		{
			return connection_handler(connection_fd);
		}

		close(connection_fd);
	}

	close(socket_fd);
	unlink("./demo_socket");

	return 0;
}