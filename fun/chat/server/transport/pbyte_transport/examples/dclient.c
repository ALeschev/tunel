#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#include "pbyte_transport.h"

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

int message_handler (pb_dialog_t *dialog, char *data, int size)
{
	// char tmp_buff[128] = {0};

	// sprintf(tmp_buff, "%s:%s", __FILE__, __func__);

	// pb_print_buff (tmp_buff, data, size);

	return 0;
}

int error_handler (pb_dialog_t *dialog, char *data, int size)
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
	char addr[] = "127.0.0.1";
	int i, port = 12345;
	char data[128] = {0};

	pb_transport_t transport;
	pb_params_t client_params;

	memset (&transport, 0, sizeof (transport));
	memset (&client_params, 0, sizeof (client_params));

	client_params.mode = ePBYTE_CLIENT;
	client_params.io_threads = 2;
	client_params.addr = addr;
	client_params.port = port;
	client_params.logger_prio = 2;

	client_params.msg_cb.client_msg_handler = &message_handler;
	client_params.err_cb.client_error_handler = &error_handler;
	client_params.logger = &logger;
	client_params.thread_init = &thread_init;

	pb_transport_start(&transport, &client_params);

	srand (time (NULL));

#if 0
	char ch = ' ';
	printf ("Press 'q' for exit: ");
	while (1)
	{
		ch = fgetc(stdin);
		if (ch == 'q')
		{
			break;
		} else
		if (ch == 's')
		{
			pb_identity_t identity = {0};

			strcpy(identity.dialog.dialog_id, "test");
			identity.dialog.dialog_len = 4;

			for (i = 0; i < sizeof (data); i++)
				data[i] = rand()%255 + 1;

			pb_transport_send (&transport, &identity, data, strlen(data));
		} else
		if ('a' <= ch && ch <= 'z')
		{
			printf ("Press 'q' for exit: ");
			usleep(100000);
		} else {
			continue;
		}
	}
#else
	pb_dialog_t dialog = {0};

	strcpy(dialog.dialog_id, "test");
	dialog.dialog_len = 4;

	while (1)
	{
		for (i = 0; i < sizeof (data); i++)
			data[i] = rand()%255 + 1;

		if (pb_transport_send (&transport, NULL, &dialog, data, strlen(data)) != 0)
			sleep (5);

		usleep(50000);
	}
#endif

	pb_transport_stop(&transport);

	return 0;
}