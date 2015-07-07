
#define MAX_NICK_LEN 64

typedef struct 
{
	int valid;
	int fd;
	int id;
	size_t nick_len;
	char nickname[MAX_NICK_LEN];
} c_client_t;

int c_init(void);
int c_client_add(int fd, char *nickname, size_t len);
int c_client_get_by_id(uint16_t id);
int c_client_get_by_fd(int fd);
int c_client_get_by_nick(char *nickname, size_t len);