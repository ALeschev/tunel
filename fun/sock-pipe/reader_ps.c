#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "frame.h"

int proc_test(int fd)
{
	int res, offset = 0;
	char buffer[MAX_BUF];
	header_t *header;
	unsigned long bytes = 0, recv_msg = 0;

	while(1)
	{
		offset = 0;
		do
		{
			res = read(fd, &buffer[offset], sizeof(header_t) - offset);
			if (res < 0)
			{
				if (errno == EINTR)
				{
					printf("header: catch write EINTR: %s\n", strerror(errno));
					continue;
				}
				if (errno == EAGAIN)
				{
					printf("header: catch write EAGAIN: %s\n", strerror(errno));
					continue;
				}
				break;
			}
			offset += res;
		} while (offset < sizeof(header_t) && res > 0);

		header = (header_t *)buffer;
		if (header->cmd == CMD_DIE)
			goto exit__;

		bytes += offset;
		recv_msg++;

		if (header->plen <= MAX_BUF - sizeof(header_t))
		{
			offset = 0;
			do
			{
				res = read(fd, &buffer[sizeof(header_t) + offset], header->plen - offset);
				if (res < 0)
				{
					if (errno == EINTR)
					{
						printf("payload: catch write EINTR: %s\n", strerror(errno));
						continue;
					}
					if (errno == EAGAIN)
					{
						printf("payload: catch write EAGAIN: %s\n", strerror(errno));
						continue;
					}
					break;
				}
				offset += res;
			} while (offset < header->plen && res > 0);
		}

		bytes += offset;

		if (res < 0)
			break;
	}

exit__:
	printf("recv %lu messages %lu bytes\n", recv_msg, bytes);

	return 0;
}

int read_sock(void)
{
	struct sockaddr_un address;
	int  socket_fd;

	socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(socket_fd < 0)
	{
		printf("socket() failed\n");
		return 1;
	}

	memset(&address, 0, sizeof(struct sockaddr_un));

	address.sun_family = AF_UNIX;
	snprintf(address.sun_path, sizeof(address.sun_path), "./demo_socket");

	if(connect(socket_fd, (struct sockaddr *) &address, sizeof(struct sockaddr_un)) != 0)
	{
		printf("connect() failed\n");
		return 1;
	}

	proc_test(socket_fd);

	close(socket_fd);
	return 0;
}

int read_pipe(void)
{
	int fd;
	char * myfifo = "./demo_fifo";

	fd = open(myfifo, O_RDONLY);

	proc_test(fd);

	close(fd);

	return 0;
}


int main(int argc, char *argv[])
{
	if (argc <= 1)
	{
		printf("set read type: %s sock\\pipe\n", argv[0]);
		return -1;
	}

	if (!strncmp(argv[1], "sock", 4))
	{
		read_sock();
	} else
	if (!strncmp(argv[1], "pipe", 4))
	{
		read_pipe();
	} else
	{
		printf("set writer type: ./%s sock\\pipe\n", argv[0]);
	}

	return 0;
}
