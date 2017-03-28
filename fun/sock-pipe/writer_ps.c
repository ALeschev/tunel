#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "frame.h"

int MSG_PER_CYCLE = 0;
int DELAY_PER_CYCLE = 0;
int TEST_TIME = 0;

unsigned long sys_tick_count_msec(void)
{
	struct timespec tp;

	clock_gettime(CLOCK_MONOTONIC, &tp);

	return (tp.tv_sec*1000 + tp.tv_nsec/1000000);
}

int start_test(int fd)
{
	int	i, res, offset = 0;
	char buffer[MAX_BUF];
	message_t *message;
	unsigned long start_time, cur_time;
	unsigned long write_time1, write_time_tot = 0;
	unsigned long bytes = 0;

	message = (message_t *)buffer;
	message->header.plen = (unsigned int)(MAX_BUF - sizeof(message_t));
	memset(message->payload, 0xFF, message->header.plen);

	cur_time = start_time = sys_tick_count_msec();

	while(cur_time - start_time < TEST_TIME)
	{
		for (i = 0; i < MSG_PER_CYCLE; i++)
		{
			message->header.msg_num++;
			offset = 0;
			do
			{
				write_time1 = sys_tick_count_msec();
				res = write(fd, &buffer[offset], MAX_BUF - offset);
				write_time_tot += sys_tick_count_msec() - write_time1;
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

	// --message->header.msg_num;
	message->header.cmd = CMD_DIE;
	message->header.plen = 0;

	write(fd, buffer, sizeof(message->header));

exit__:
	printf("send %lu messages; %lu bytes; time per write %.3fms\n",
	         message->header.msg_num, bytes,
	         (float)write_time_tot/(float)message->header.msg_num);
	return 0;
}

int writer_sock(void)
{
	struct sockaddr_un address;
	int socket_fd, connection_fd;
	socklen_t address_length;

	socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(socket_fd < 0)
	{
		printf("socket() failed\n");
		return 1;
	}

	setsockopt(selfmodule->listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	int sendbuf = 2 * 1024;
	int sendbuflen = sizeof(sendbuf);
	int recvbuf = 2 * 1024;
	int recvbuflen = sizeof(recvbuf);

	setsockopt(selfmodule->listenfd, SOL_SOCKET, SO_SNDBUF, &sendbuf, sendbuflen);
	setsockopt(selfmodule->listenfd, SOL_SOCKET, SO_RCVBUF, &recvbuf, recvbuflen);

	unlink("./demo_socket");

	memset(&address, 0, sizeof(struct sockaddr_un));

	address.sun_family = AF_UNIX;
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
		start_test(connection_fd);
		close(connection_fd);
		break;
	}

	close(socket_fd);
	unlink("./demo_socket");

	return 0;
}

int writer_pipe(void)
{
	int	fd;
	char * myfifo = "./demo_fifo";

	mkfifo(myfifo, 0666);

	fd = open(myfifo, O_WRONLY);

	start_test(fd);

	close(fd);
	unlink(myfifo);

	return 0;
}

int main(int argc, char *argv[])
{
	if (argc <= 4)
	{
		printf("set writer type: %s sock\\pipe cycle_msg delay[ms] test_time[s]\n", argv[0]);
		return -1;
	}

	MSG_PER_CYCLE = atoi(argv[2]);
	DELAY_PER_CYCLE = atoi(argv[3]) * 1000;
	TEST_TIME = atoi(argv[4]) * 1000;

	printf("header size %u; payload size %u; tot %u\n",
	       sizeof(header_t), MAX_BUF - sizeof(header_t), MAX_BUF);

	if (!strncmp(argv[1], "sock", 4))
	{
		writer_sock();
	} else
	if (!strncmp(argv[1], "pipe", 4))
	{
		writer_pipe();
	} else
	{
		printf("set writer type: ./%s sock\\pipe\n", argv[0]);
	}

	return 0;
}
