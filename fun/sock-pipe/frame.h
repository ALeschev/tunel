#ifndef __FRAME_H__
#define __FRAME_H__

// #define MAX_BUF 12400
#define MAX_BUF 16

#define CMD_DIE 332

// #define MSG_PER_CYCLE 100000
// #define DELAY_PER_CYCLE 50000

// #define TEST_TIME 60000

typedef struct
{
	int cmd;
	unsigned long msg_num;
	unsigned int plen;
} header_t;

typedef struct
{
	header_t header;
	char payload[0];
} message_t;



#endif