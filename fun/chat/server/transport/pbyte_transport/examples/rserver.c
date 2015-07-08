#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>

#include "pbyte_transport.h"

pb_transport_t transport;

void pb_print_buff (const char *descript, char *data, int size)
{
	int i;
	char out_buffer[64] = {0};
	char tmp_buff[64] = {0};

	if (descript && strlen(descript))
		printf ("%s:\n", descript);

	if (!data || size == 0)
	{
		printf ("     (empty)\n");
		return;
	}

	for (i = 0; i < size; i++)
	{
		if (!(i%16))
		{
			if (i != 0)
			{
				printf("%s\n", out_buffer);
			}
			sprintf (out_buffer, "%d: ", i);
		}
		sprintf(tmp_buff, "%02X ", (unsigned)data[i]);
		strcat(out_buffer, tmp_buff);
	}
	if (i % 16)
	{
		strcat(out_buffer, tmp_buff);
	}

	printf("%s\n", out_buffer);
	printf("Total: %d bytes\n", size);
}

int message_handler (pb_identity_t *identity, char *data, int size)
{
	// char tmp_buff[128] = {0};

	// sprintf(tmp_buff, "%s:%s", __FILE__, __func__);

	// pb_print_buff (tmp_buff, data, size);

	pb_transport_send (&transport, identity, NULL, data, size);

	return 0;
}

int error_handler (pb_identity_t *identity, char *data, int size)
{
	// char tmp_buff[128] = {0};

	// sprintf(tmp_buff, "%s:%s", __FILE__, __func__);

	// pb_print_buff (tmp_buff, data, size);

	return 0;
}

void logger (int prio, char *format, ...)
{
	va_list	ap;

	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
}

void thread_init (void)
{
	printf ("%s: thread inited\n", __FILE__);
}

int main (void)
{
	char ch = ' ';
	char addr[] = "127.0.0.1";
	int port = 12345;

	pb_params_t server_params;

	memset (&transport, 0, sizeof (transport));
	memset (&server_params, 0, sizeof (server_params));

	server_params.mode = ePBYTE_SERVER;
	server_params.io_threads = 2;
	server_params.addr = addr;
	server_params.port = port;
	server_params.logger_prio = 2;

	server_params.msg_cb.server_msg_handler = &message_handler;
	server_params.err_cb.server_error_handler = &error_handler;
	server_params.logger = &logger;
	server_params.thread_init = &thread_init;

	pb_transport_start(&transport, &server_params);

	printf ("Press 'q' for exit: ");
	while (1)
	{
		ch = fgetc(stdin);
		if (ch == 'q')
		{
			break;
		} else
		if ('a' <= ch && ch <= 'z')
		{
			printf ("Press 'q' for exit: ");
			usleep(100000);
		} else {
			continue;
		}
	}

	pb_transport_stop(&transport);

	return 0;
}