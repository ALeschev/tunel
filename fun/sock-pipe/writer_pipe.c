#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "frame.h"

unsigned long sys_tick_count_msec(void)
{
	struct timespec tp;

	clock_gettime(CLOCK_MONOTONIC, &tp);

	return (tp.tv_sec*1000 + tp.tv_nsec/1000000);
}

int main()
{
	int	i, fd, res, offset = 0;
	char * myfifo = "./demo_fifo";
	char buffer[MAX_BUF];
	message_t *message;
	unsigned long start_time, cur_time;
	unsigned long bytes = 0;

	mkfifo(myfifo, 0666);

	fd = open(myfifo, O_WRONLY);

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
				res = write(fd, &buffer[offset], MAX_BUF - offset);
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

	bytes += write(fd, buffer, sizeof(message->header));

	printf("send %lu messages %lu bytes\n", message->header.msg_num, bytes);

exit__:
	close(fd);

	/* remove the FIFO */
	unlink(myfifo);

	return 0;
}
