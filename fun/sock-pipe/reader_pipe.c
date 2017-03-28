#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "frame.h"

int proc_test(int fd)
{
	int fd, res, offset = 0;
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
		bytes += offset;
		recv_msg++;
		if (header->cmd == CMD_DIE)
			goto exit__;

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

int main()
{
	char * myfifo = "./demo_fifo";

	fd = open(myfifo, O_RDONLY);

	proc_test(fd);

	close(fd);

	return 0;
}
