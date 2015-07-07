
enum e_command
{
/*0*/ eCOMMAND_NONE = 0,

/*1*/ eCOMMAND_INIT_CONNECTION,
/*2*/ eCOMMAND_CLOSE_CONNECTION,

/*3*/ eCOMMAND_PING,
/*4*/ eCOMMAND_PONG,

/*5*/ eCOMMAND_SET_NICKNAME,
/*6*/ eCOMMAND_SEND_BROADCAST,

/*7*/ eCOMMAND_MAX
};

typedef struct
{
	uint16_t command;
	uint16_t from_id;
	uint16_t to_id;
	uint16_t payload_len;
} m_header_t;

typedef struct
{
	m_header_t header;
	uint8_t payload[0];
} m_message_t;

m_message_t *m_message_init(m_header_t *header, uint8_t *data, size_t len);
int m_message_close(m_message_t *message);