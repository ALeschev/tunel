
#include "version.h"
#include "proc.h"
#include "que.h"
#include "bcktrace.h"
#include "config.h"
#include "cli/libcli.h"
#include "cli/cmdUserFunc.h"
#include "modules.h"
#include "service_f.h"

#ifdef _CFG_CONTAINER_
#include "cfgo_proc.h"
#endif

mque_t con_que;
static int con_wr_sock = -1;
#define CON_SOCK_NAME "con_cfg"

ut_thread_t *consol_thread=NULL;
void	    *consol_task(void *);

ut_thread_t *consol_thread_wr=NULL;
void	    *consol_task_wr(void *);

struct cli_command *c;
struct cli_def *cli = NULL;
socklen_t clilen;
struct sockaddr_in cliaddr;

volatile int con_socket=-1;
volatile int con_connid=-1;
volatile int con_loop=0;
int module_reg = 0;

void consol_close(void);

void mspcSetTrace(int lvl);
int SetMspTrace(int idx, int lvl);
int SetTraceErlL1(int lvl);
int SetTraceErlL3(int lvl);

#ifdef _USE_YAML_

int SetTraceCfg(int lvl);
int SetTraceParse(int lvl);
int SetTraceCfgO(int lvl);
#endif


int SetTracePeer(int lvl);
int SetTraceHandlers(int lvl);
int SetTraceChanges(int lvl);
int treap_SetTrace(int lvl);

//------------------------------------------------------------

static int getServerSocket(int port)
{
	int i, listenSocket;
	struct sockaddr_in listenSockaddr;

	memset(&listenSockaddr, 0, sizeof(listenSockaddr));

	if((listenSocket=socket(PF_INET,SOCK_STREAM,0))<0)
	{
		app_trace(TRACE_ERR, "Consol: cannot open socket");
		return -1;
	}

	i = 1;
	setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(int));
	//setsockopt(listenSocket, SOL_SOCKET, SO_REUSEPORT, &i, sizeof(int));


	listenSockaddr.sin_family = PF_INET;
	listenSockaddr.sin_port = htons(port);
	listenSockaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	if(bind(listenSocket,(struct sockaddr*)&listenSockaddr, sizeof(listenSockaddr)) < 0)
	{
		app_trace(TRACE_ERR, "Consol: cannot bind socket. error: '%s'", strerror(errno));
		return -2;
	}

	return listenSocket;
}


static ssize_t sock_readline(int sockfd, void *vptr, size_t maxlen)
{
	ssize_t n, rc;
	char c, *ptr;

	ptr = vptr;

	for(n=1; n<maxlen; n++)
	{
	   again:
		if((rc=read(sockfd, &c, 1)) == 1)
		{
			if(c == '\n') break;
			if(c == '\r') continue;

			*ptr++ = c;
			continue;
		}

		if(rc == 0)
		{
			if(n == 1) return 0;	/* EOF, no data */
			break;			/* EOF + some data */
		}


		if(errno == EINTR) goto again;

		//app_trace(TRACE_ERR, "Consol: sock read error %s", strerror(errno));
		app_trace(-1, "Consol: sock read error %s", strerror(errno));

		return -1;			/* error */
	}

	*ptr = 0;
	return n;
}

int c_gets(void *line, size_t size)
{
	return sock_readline(con_connid, line, size);
}


void c_puts(char *str)
{
	cli_print(cli, str);

	return;
#if 0
	uint8_t buf[8192];

	if(con_wr_sock < 0) return;

	int len = strlen(str);
	memcpy(buf, str, len);

	buf[len++] = 0;

	//printf("con: add que buf. %d\r\n", len);
#ifndef MEMDBG
	int sz;
	if( (sz = add_que(&con_que, buf, 1)) > 0)	//������������ ���������� �������
#endif
	{
		{
		        struct sockaddr_un addr = {0};
		        int res;

			sprintf(addr.sun_path, "/var/run/%s", CON_SOCK_NAME);
			addr.sun_family = AF_UNIX;
			int s_len=sizeof(addr.sun_family) + strlen(addr.sun_path);

			res = sendto(con_wr_sock, &buf, len, MSG_DONTWAIT, (struct sockaddr *) &addr, s_len);
		}
	}

	return;
#endif
}

void c_puts_wr(char *str)
{
    cli_print(cli, str);
}

extern struct tm start_time;
extern unsigned long start_time_t;

int sysStart(char *str)
{
	//start_time_t

	sprintf(str, "# System started at:\t%02i.%02i.%02i  %02i:%02i:%02i\n",
		start_time.tm_mday, start_time.tm_mon+1, start_time.tm_year % 100, start_time.tm_hour,
		start_time.tm_min, start_time.tm_sec);

	return 0;
}

int sysUptime(char *str)
{
	time_t  curtime;

	curtime = sys_tick_count_sec();

	int uptime_sec = curtime - start_time_t;

	int hour, min, sec, days;

	days = uptime_sec/(3600*24);
	hour = (uptime_sec - 3600*24*days)/3600;
	min = (uptime_sec - 3600*24*days - 3600*hour)/60;
	sec = (uptime_sec - 3600*24*days - 3600*hour - 60*min);

	sprintf(str, "# System uptime:\t%02id  %02ihour %02imin %02isec\n",
														days, hour, min, sec);

	return 0;
}

int sysTime(char *str)
{
	time_t  curtime;
	struct tm tm;

	time(&curtime);
	localtime_r(&curtime, &tm);

	sprintf(str, "# System time:\t\t%02i.%02i.%02i  %02i:%02i:%02i\n",
		tm.tm_mday, tm.tm_mon+1, tm.tm_year % 100, tm.tm_hour,
		tm.tm_min, tm.tm_sec);

	return 0;
}


extern char *making_date;
extern char *making_time;

int sysFunc(void)
{
	char str[1024];
	int ptr = 0;

	sprintf(str, "\n=====================================================\n\n");
	ptr = strlen(str);

	sprintf(&str[ptr], "# Software Version: V.0.%u.%u.%sBuild: %s %s\n",
		VERSION, SUBVERSION,
#ifdef _SIP_USERS_
		" [SIPu] ",
#else
		" ",
#endif
		making_date, making_time);

	ptr = strlen(str);

	sprintf(&str[ptr], "\n");
	ptr = strlen(str);

	sysTime(&str[ptr]);
	ptr = strlen(str);
	sprintf(&str[ptr], "\n");

	sysStart(&str[ptr]);
	ptr = strlen(str);
	sprintf(&str[ptr], "\n");

	sysUptime(&str[ptr]);
	ptr = strlen(str);

	sprintf(&str[ptr], "\n=====================================================\n\n");

	c_puts(str);

	return 0;
}

typedef struct  {
	char *name;
	int id;
	int param;
} t_trace_f;


typedef enum {
	CMD_OFF,
	CMD_START,
	CMD_STOP,
	CMD_SHOW,
	CMD_LAPM,
	CMD_PKT,
	CMD_SLOT,
	CMD_STREAM,
	CMD_CASL3,
	CMD_PRIL3,
	CMD_PORT,
	CMD_ISUP,
	CMD_MTP3,

	CMD_SIPT,
	CMD_MSPC,
	CMD_MSPD,
	CMD_ERL_L1,
	CMD_ERL_L3,
#ifdef MEMDBG
	CMD_UT_MEM,
#endif

	CMD_ADMNET,
	CMD_ADMIN,
	CMD_PEER,
	CMD_HANDLERS,
	CMD_CFG_CHANGE,
	CMD_RSV,
	CMD_SORM,
	CMD_ALARM,
	CMD_SWITCH,
	CMD_HWPKT,

	CMD_PASS,
	CMD_HOTNUM,
	CMD_REDIR,
	CMD_SHORT,
	CMD_SRV,
	CMD_SRVNET,
	CMD_PROXY,
	CMD_STAT,
	CMD_RADIUS,
	CMD_RADIUS_CHAIN,

	CMD_CFG,
	CMD_CFGO,
	CMD_TREAP,
	CMD_PARSE,
	CMD_SERVICE
} cmd_enum;


t_trace_f trace_f[] = {
	{"off",    CMD_OFF,   0},
	{"start",  CMD_START, 0},
	{"stop",   CMD_STOP,  0},
#if 0
	{"show",   CMD_SHOW,  0},
	{"lapm",   CMD_LAPM,  2},
	{"pkt",    CMD_PKT,   2},
	{"slot",   CMD_SLOT,  2},
	{"stream", CMD_STREAM,  3},
	{"casl3", CMD_CASL3,  3},
	{"pril3", CMD_PRIL3,  3},
	{"port",   CMD_PORT,  2},
	{"isup",   CMD_ISUP,  1},

	{"sipt",   CMD_SIPT,  1},
	{"mspd",   CMD_MSPD,  2},
	{"mspc",   CMD_MSPC,  1},

	{"erl1",   CMD_ERL_L1,  1},
	{"erl3",   CMD_ERL_L3,  1},
#ifdef MEMDBG
	{"utmem",  CMD_UT_MEM,  1},
#endif

	{"mtp3",   CMD_MTP3,  2},
	{"admnet", CMD_ADMNET,1},
	{"admin",  CMD_ADMIN, 1},

	{"rsv",    CMD_RSV,   1},
	{"sorm",   CMD_SORM,  1},
	{"alarm",  CMD_ALARM, 1},
	{"sw",	   CMD_SWITCH,1},
	{"hwpkt",  CMD_HWPKT, 2},

	{"pass",  CMD_PASS, 2},
	{"hotnum",  CMD_HOTNUM, 2},
	{"redir",  CMD_REDIR, 3},
	{"short",  CMD_SHORT, 3},
	{"srv",  CMD_SRV, 1},
	{"srvnet",  CMD_SRVNET, 1},
	{"proxy",  CMD_PROXY, 1},
	{"stat",  CMD_STAT, 1},

	{"radius",  CMD_RADIUS, 1},
	{"rchain",  CMD_RADIUS_CHAIN, 1},

#endif //0
	{"cfg",  CMD_CFG, 1},
	{"cfgo",  CMD_CFGO, 1},
	{"treap",  CMD_TREAP, 1},

	{"parse",  CMD_PARSE, 1},
	{"peer",   CMD_PEER,  1},
	{"handl",   CMD_HANDLERS,  1},
	{"change",  CMD_CFG_CHANGE,  1},
	{"service", CMD_SERVICE,  1},

	{"", -1}
};

#define TPT_NONE	0
#define TPT_CAS		1
#define TPT_PRI		2
#define TPT_SS7		3
#define TPT_SS7_LS	4
#define TPT_V52		5
#define TPT_V52_I	6
#define TPT_ALL		7
#define TPT_NUM		8

int SetMTP3Trace(int line, int lvl);
int SetTraceHwPkt(int idx, int lvl);

void trace(char *param)
{
	char *p, str[2048];
	t_trace_f *ptf;
	int r, val=0, val2, idx, lvl;
	char onoff[20];


	while(*param && (*param <= ' ')) param++;

	if(!*param)
	{
		c_puts("trace point arg error\n");
/*
		c_puts(	"     Using trace:\n"
		        "           trace point on/off idx  - enable/disable trace\n"
			"           point: \n"
			"                  - hwpkt  - show l1 data-exchange MCCP <-> M16E1\n"
			"                  - lapm   - show l2 data-exchange MCCP <-> M16E1\n"
			"                  - slot   - show l3 data-exchange MCCP <-> M16E1\n"
			"                  \n"
			"                  - port   - show port-activity traces\n"
			"                  - isup   - show ISUP-activity traces\n"
			"                  - casl3  - show CAS-activity traces\n"
			"                  - pril3  - show PRI-activity traces\n"
			"                  \n"
			"                  - admnet - show l1 data-exchange MCCP <-> MTX\n"
			"                  - admin  - show l3 data-exchange MCCP <-> MTX\n"
			"                  - peer   - show data-exchange with peer\n"
			"                  - rsv    - show reserv information\n"
			"                  - alarm  - show alarms activity\n");
*/
		return;
	}

	p = param;
	while(*param && (*param > ' ')) param++;
	if(*param) *param++ = 0;

	r = 0;
	val = 0;
	val2 = 0;
	lvl = 0;

	for(ptf=trace_f; ptf->id>=0; ptf++)
	{
		if(!strcasecmp(ptf->name, p)) break;
	}

	if(ptf->id < 0)
	{
		c_puts("trace point syntax error\n");
/*
		c_puts(	"     Using trace:\n"
		        "           trace point on/off idx  - enable/disable trace\n"
			"           point: \n"
			"                  - hwpkt  - show l1 data-exchange MCCP <-> M16E1\n"
			"                  - lapm   - show l2 data-exchange MCCP <-> M16E1\n"
			"                  - slot   - show l3 data-exchange MCCP <-> M16E1\n"
			"                  \n"
			"                  - port   - show port-activity traces\n"
			"                  - isup   - show ISUP-activity traces\n"
			"                  - casl3  - show CAS-activity traces\n"
			"                  - pril3  - show PRI-activity traces\n"
			"                  \n"
			"                  - admnet - show l1 data-exchange MCCP <-> MTX\n"
			"                  - admin  - show l3 data-exchange MCCP <-> MTX\n"
			"                  - peer   - show data-exchange with peer\n"
			"                  - rsv    - show reserv information\n"
			"                  - alarm  - show alarms activity\n"
			"                  - radius - show RADIUS requests/replies\n"
			);
*/
		return;
	}

	if(ptf->param)
	{
		r = sscanf(param, "%s %d %d %d", onoff, &idx, &val2, &lvl);

		if(r < ptf->param)
		{
			sprintf(str, "syntax error (cmd param %d, need %d)\n", r, ptf->param);
			c_puts(str);
			return;
		}else{
			if(r < 2) idx = 0;
			if(r < 3) val2 = 0;
			if(r < 4) lvl = 0;
		}
	}

	if(r)
	{
		if(!strcasecmp(onoff, "on"))  val = 1;
		else
		if(!strcasecmp(onoff, "off")) val = 0;
		else
		{
			c_puts("trace point value (on/off) error\n");
			return;
		}
	}

	switch(ptf->id) {
	   case CMD_OFF:
		ClearAllTrace();
		c_puts("Stop trace and clear all object trace\n");
		return;

	   case CMD_START:
		c_puts("Start trace\n");
		StartTrace();
		return;

	   case CMD_STOP:
		StopTrace();
		c_puts("Stop trace\n");
		return;

	   case CMD_SHOW:
		StateTrace(str);
		c_puts(str);
		return;
#ifdef _USE_YAML_
	   case CMD_CFG:
		val = SetTraceCfg(idx ? idx : val);
		sprintf(str, "set cfg-trace. %s (%d)", val ? "on":"off", val);
		c_puts(str);
		return;
	   case CMD_CFGO:
		val = SetTraceCfgO(idx ? idx : val);
		sprintf(str, "set cfgo-trace. %s (%d)", val ? "on":"off", val);
		c_puts(str);
		return;
	   case CMD_TREAP:
		val = treap_SetTrace(idx ? idx : val);
		sprintf(str, "set treap-trace. %s (%d)", val ? "on":"off", val);
		c_puts(str);

	   	return;
	   case CMD_PARSE:
		val = SetTraceParse(idx ? idx : val);
		sprintf(str, "set parse-trace. %s (%d)", val ? "on":"off", val);
		c_puts(str);
		return;
#endif
	   case CMD_PEER:
		val = SetTracePeer(idx ? idx : val);
		sprintf(str, "set peer-trace. %s (%d)", val ? "on":"off", val);
		c_puts(str);
		return;

	   case CMD_HANDLERS:
		val = SetTraceHandlers(idx ? idx : val);
		sprintf(str, "set peer-handlers. %s (%d)", val ? "on":"off", val);
		c_puts(str);
		return;

	   case CMD_CFG_CHANGE:
		val = SetTraceChanges(idx ? idx : val);
		sprintf(str, "set cfg-chages. %s (%d)", val ? "on":"off", val);
		c_puts(str);
		return;

		case CMD_SERVICE:
		val = SetTraceService(idx ? idx : val);
		sprintf(str, "set service-trace. %s (%d)", val ? "on":"off", val);
		c_puts(str);
		return;

	}
}

/***************************************************************************/

#ifdef MEMDBG
	V5_COMMAND_HANDLER(memdump)
		int num = -1, mode = 1, minsize = 0 ;


		if(argc > 0)
		{
			if(!strcmp(argv[0], "big"))
			{
				minsize = 350000; /*350kb*/
			}else{
				num = atoi(argv[0]);

				if(argc > 1)
				{
					mode = atoi(argv[1]);
				}
			}
		}

		UT_DumpMemInfoEx(num, mode, minsize);

	END_HANDLER(memdump)
#endif
/***************************************************************************/

/***************************************************************************/

	V5_COMMAND_HANDLER(hwreboot)
		app_trace(TRACE_INFO, "System will now reboot...");
		app_pause(500);
		system("reboot");
	END_HANDLER(hwreboot)
/***************************************************************************/

/***************************************************************************/
	V5_COMMAND_HANDLER(trace)
    	trace(param);
	END_HANDLER(trace)
/***************************************************************************/
	V5_COMMAND_HANDLER(stop)

		app_trace(TRACE_INFO, "command 'stop'");

		app_snd_msg(MSG_SHUTDOWN, 0, NULL);

		ClearAllTrace();

		unregister_log();

		con_loop = 0;
		if(con_connid >= 0)
		{
			shutdown(con_connid, 2);
			close(con_connid);
			con_connid = -1;
		}
	END_HANDLER(stop)


/***************************************************************************/
	V5_COMMAND_HANDLER(sys)
    	sysFunc();
	END_HANDLER(sys)
/***************************************************************************/
	V5_COMMAND_HANDLER(regcon)
    	register_log(c_puts);
	END_HANDLER(regcon)
/***************************************************************************/
	V5_COMMAND_HANDLER(unregcon)
    	unregister_log();
	END_HANDLER(unregcon)

/***************************************************************************/

	V5_COMMAND_HANDLER(count)
    	int type = 0;
    	int r = sscanf(param, "%d", &type);
	if(r < 1)
	{
		c_puts("param error (type)");
	}else{
		r = cfg_get_count(type);
#ifdef _CFG_CONTAINER_
		r = cfgo_GetObjectCount(type);
#endif
		app_trace(TRACE_INFO, "type [%d], count <%d>", type, r);
	}
	END_HANDLER(count)
/***************************************************************************/
#ifdef _CFG_CONTAINER_
int cfgo_ShowObjectByIndex(enum cfg_objects objectType, int index);
#endif

	V5_COMMAND_HANDLER(show)
	int type = 0, idx = -1;
	int r = sscanf(param, "%d %d", &type, &idx);
	if(r < 2)
	{
		c_puts("param error (type, idx)");
	}else{
#ifdef _CFG_CONTAINER_
  if(cfgo_GetProcessing(type)) cfgo_ShowObjectByIndex(type, idx);
  	else
  		app_trace(TRACE_INFO, "Info:\r\n  failed to get info!");
#else
  {
    char info_str[2048];
    cfg_get_obj_info(type, idx, info_str);
    app_trace(TRACE_INFO, "Info:\r\n  %s", info_str);
  }
#endif


	}
	END_HANDLER(show)
/***************************************************************************/
	V5_COMMAND_HANDLER(set)

	int type = 0, idx = 0;
	int r = sscanf(param, "%d %d", &type, &idx);
	if(r < 2)
	{
		c_puts("param error. need: [type] [idx]");
		return 0;
	}

#ifndef _CFG_CONTAINER_
	{
		uint8_t data[512];
		int size = config_data_size(type);

		//int count = cfg_get_count(type);

		data[0] = (size >> 8) & 0xFF;
		data[1] = (size)& 0xFF;

		if(size)
		{
			r = cfg_set_data(type, idx, data, size+2, 1, 0);
		}
		app_trace(TRACE_INFO, "type [%d], idx [%d]. set-result <%d>", type, idx, r);
	}
#else

	const char *res_err;
	uint8_t *data = NULL;

	const cfgo_proc_t * proc = cfgo_GetProcessing(type);

	if(!proc)
	{
		c_puts("failed to get 'proc' data for object");
		return 0;
	}

	if(proc->dataSize) 
	{
		data = ut_malloc_ex(proc->dataSize);
		memset(data, 0, proc->dataSize);
	}

	r = cfgo_SetObjectParams(type, idx, data, proc->dataSize, NULL);

	res_err = cfgerr_to_string(r);

	app_trace(TRACE_INFO, "type [%d], idx [%d]. set-result <%d> %s", type, idx, r, res_err);

	if(proc->dataSize) ut_free_ex(data);

#endif

	END_HANDLER(set)

/***************************************************************************/
	V5_COMMAND_HANDLER(rem)
	int type = 0, idx = 0;
	int r = sscanf(param, "%d %d", &type, &idx);
	if(r < 2)
	{
		c_puts("param error (type, idx)");

		return 0;
	}

#ifndef _CFG_CONTAINER_
	{
		r = cfg_rem_data(type, idx, 0);

		app_trace(TRACE_INFO, "type [%d], idx [%d]. rem-result <%d>", type, idx, r);
	}
#else
	int ID;
	cfgo_data_t *data = NULL;
	const cfgo_proc_t * proc = NULL;
	data = cfgo_GetObjectDataByIndex(type, idx);
	proc = data ? data->proc : NULL;
	ID = data ? data->id : 0;

	if(!data)
	{
		r = CFGERR_CFGO_FAILED_TO_GET_DATA;

		goto rem_exit;
	}

	r = cfgo_RemObjectData(type, idx, data);
	cfg_post_remove_processing(type, idx, ID, NULL);
	config_info_removed(type, idx, 0);

rem_exit:
	app_trace(TRACE_INFO, "type [%d], idx [%d]. set-result <%d> %s", type, idx, r, cfgerr_to_string(r));

#endif
	END_HANDLER(rem)
/***************************************************************************/
#ifdef _USE_YAML_
int config_read(int mode, int uploaded);
int config_write(int tmp);

	V5_COMMAND_HANDLER(cfg_read)
		config_read(0, 0);
		if(ena_sip_uservice) service_file_read(0);
	END_HANDLER(cfg_read)

	V5_COMMAND_HANDLER(cfg_write)
    	config_write(1);
	END_HANDLER(cfg_write)

	V5_COMMAND_HANDLER(srv_apply)
		int tmp = 0;
		if(strcmp(param, "running"))
		{
			tmp = 1;
		}
		else
		if(strcmp(param, "startup"))
		{
			tmp = 1;
		}
		else
		{
			c_puts("param error (cfg type 'running'/'startup')");
			return 0;
		}

		int res = service_file_read(tmp);
		cfg_set_modify(CF_MOD);
		app_trace(TRACE_INFO, "CFG_SRV_FILE_APPLY res: '%s'[%d]", cfgerr_to_string(res), res);
	END_HANDLER(srv_apply)

#endif

/***************************************************************************/

	V5_COMMAND_HANDLER(ip_test)
    	char ip_str[64];
    	uint8_t ip[4];

    	sprintf(ip_str, "192.168.138.122");

    	app_trace(TRACE_INFO, "ip: %s.", ip_str);

    	sscanf(ip_str, "%hhd.%hhd.%hhd.%hhd", &ip[0], &ip[1], &ip[2], &ip[3]);

    	app_trace(TRACE_INFO, "ip: %s. %d.%d.%d.%d", ip_str, ip[0], ip[1], ip[2], ip[3]);


    	int ip_int = (ip[0]<<24) | (ip[1]<<16) | (ip[2]<<8) | ip[3];

    	int ip_int_inet_net = inet_addr(ip_str);
    	int ip_int_inet_host = ntohl(inet_addr(ip_str));

    	app_trace(TRACE_INFO, "ip_interger: %x. <%s>", ip_int, ip2str(ip_int, 0));
    	app_trace(TRACE_INFO, "ip_inet_net: %x. <%s>", ip_int_inet_net, ip2str(ip_int_inet_net, 0));
    	app_trace(TRACE_INFO, "ip_inet_hst: %x. <%s>", ip_int_inet_host, ip2str(ip_int_inet_host, 0));

	END_HANDLER(ip_test)

/***************************************************************************/
	V5_COMMAND_HANDLER(save)
		system("save");
	END_HANDLER(save)
/***************************************************************************/

	V5_COMMAND_HANDLER(registration)

	char *answer = NULL;
	message_header_t *answer_header = NULL;
	int result = 0;
	int module = 0;

	module = (CONFMOD << 16);

	if (send_message(WD_REGISTER, module, 0, MESSAGE_TYPE_REQUEST, WDMOD,
		0, 0, NULL, &answer, CONFMOD_REPLY_TO*2) < 0)
	{
		app_trace(TRACE_ERR, "ERROR: Can't send REGISTRATION to WDMOD\n");
	} else {
		answer_header = (message_header_t *)answer;
		if (!answer_header || (answer_header->rcode != CFGERR_OK) ) {
			app_trace(TRACE_ERR, "ERROR: WD REGISTRATION answer is not OK!\n");
		}else{
			result = 1;
		}
	}

	if(result) module_reg = 1;

	return result;

	END_HANDLER(registration)
/***************************************************************************/

	V5_COMMAND_HANDLER(deregistration)

	char *answer = NULL;
	message_header_t *answer_header = NULL;
	int result = 0;
	int module = 0;

	module = (CONFMOD << 16);

	if (send_message(WD_DEREGISTER, module, 0, MESSAGE_TYPE_REQUEST, WDMOD,
		0, 0, NULL, &answer, CONFMOD_REPLY_TO*2) < 0)
	{
		app_trace(TRACE_ERR, "ERROR: Can't send DEREGISTRATION to WDMOD\n");
	} else {
		answer_header = (message_header_t *)answer;
		if (!answer_header || (answer_header->rcode != CFGERR_OK) ) {
			app_trace(TRACE_ERR, "ERROR: WD DEREGISTRATION answer is not OK!\n");
		}else{
			result = 1;
		}
	}

	if(result) module_reg = 0;

	return result;

	END_HANDLER(deregistration)
/***************************************************************************/



void cli_reg_cmd(char *command, int (*callback)(struct cli_def *, char *, char **, int), char *help)
{
	if(cli != NULL)
		cli_register_command(cli, NULL, command, callback, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, help);
}

void cli_reg_all_cmd(void)
{
	cli_reg_cmd("save", 	cmd_save, 	"Save config");

	cli_reg_cmd("ip_test", 	cmd_ip_test, 	" Test ip-decoding");
	cli_reg_cmd("registration", cmd_registration,   "WD registration");
	cli_reg_cmd("deregistration", cmd_deregistration,   "WD deregistration");


#ifdef _USE_YAML_
	cli_reg_cmd("cfg_read", 	cmd_cfg_read, 	" Test cfg-read");
	cli_reg_cmd("cfg_write", 	cmd_cfg_write, 	" Test cfg-write");

#endif
	cli_reg_cmd("show", 		cmd_show, 	"Show objects info");
	cli_reg_cmd("count", 		cmd_count, 	"Show objects count");
	cli_reg_cmd("set", 		cmd_set, 	"Set new obj");
	cli_reg_cmd("rem", 		cmd_rem, 	"Remove obj");

#ifdef MEMDBG
	cli_reg_cmd("memdump", 		cmd_memdump, 	"Show memory-dbg info");
#endif
	cli_reg_cmd("sys", 		cmd_sys, 	"Show system info");
	cli_reg_cmd("hwreboot", 	cmd_hwreboot, 	"Hardware reboot");

	cli_reg_cmd("trace", 		cmd_trace, 	"Trace func");
	cli_reg_cmd("regcon", 		cmd_regcon, 	"Show traces in console");
	cli_reg_cmd("unregcon", 	cmd_unregcon, 	"Show traces in COM-port");
	cli_reg_cmd("stop", 		cmd_stop, 		"stop application");

	cli_reg_cmd("srv_apply", 	cmd_srv_apply, 		"Apply(reread) services");
}

int con_sock_init(char *name)
{
	struct sockaddr_un *addr, saddr;
        int so_option = 1;
        int res;
	int sock;

	addr = &saddr;

	memset(addr, 0, sizeof(struct sockaddr_un));

	sprintf(addr->sun_path, "/var/run/%s", name);

	unlink(addr->sun_path);

	addr->sun_family = AF_UNIX;
	int s_len=sizeof(saddr.sun_family) + strlen(saddr.sun_path);
	//addr->sun_path[0] = 0;

	sock = socket(AF_UNIX, SOCK_DGRAM, 0);

	if ( sock == -1 )
	{
		app_trace(TRACE_ERR, "sock: '%s' create error %s\n", name, strerror(errno));
		return -1;
	}

	res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &so_option, sizeof(so_option));

	if ( res == -1 )
	{
		app_trace(TRACE_ERR, "sock: %s error setsockopt %s\n", name, strerror(errno));
		return -1;
	}

	res = bind(sock, (struct sockaddr *) addr, s_len);

	if ( res == -1 )
	{
		app_trace(TRACE_ERR, "sock: %s error bind: %s\n", name, strerror(errno));
		return -1;
	}

	/*app_trace(TRACE_INFO, "con sock: open %s sock = %d\n", name, sock);*/

	return sock;
}

int consol_start(void)
{
	con_que.head = NULL;
	con_que.que_size = 0;

	app_trace(TRACE_INFO, "console: init");

	if((con_que.mutex=ut_mutex_init()) == NULL)
	{
		printf("console: mutex create error\r\n");
		return -1;
	}

	if((con_wr_sock=con_sock_init(CON_SOCK_NAME)) == -1)
	{
		printf("console: socket create error\r\n");
		return -1;
	}

	consol_thread = (ut_thread_t*)ut_calloc(1,sizeof(ut_thread_t));
	ut_thread_create(UT_THREAD_STACK_SIZE/4,consol_thread, consol_task, NULL);

//	consol_thread_wr = (ut_thread_t*)ut_calloc(1,sizeof(ut_thread_t));
//	ut_thread_create(UT_THREAD_STACK_SIZE/4,consol_thread_wr, consol_task_wr, NULL);

	return 0;
}

void *consol_task_wr(void *ptr)
{
	app_trace(TRACE_INFO, "consol task_wr: start. pid %i", get_ptid());

	char proc_str[64];
	sprintf(proc_str, "[%i] consol-wr task", get_ptid());
	add_proc_info(proc_str);

	app_bk_pid(get_ptid(), PID_TYPE_CON_WR, 0, 0);

	int pt = (PID_TYPE_CON_WR<<24);

	int state_q = 0;

	uint8_t buf[8192];
	int len = 0 ,rxlen = 0;
	int ev;
	fd_set rfds;
	struct timeval timeout;

	while(con_loop)
	{
		if(con_wr_sock < 0)
		{
			consol_close();
			break;
		}

		FD_ZERO (&rfds);
		FD_SET (con_wr_sock, &rfds);

		timeout.tv_sec	= 1L;
		timeout.tv_usec	= 0L;

		len = 0;

		ev = select(con_wr_sock+1, &rfds, 0, 0, &timeout);

//		app_pause(100);

		app_bk_trace(0x01 + pt, ev, 0, 0);

		state_q = 1;

/*		while(state_q)
		{
			len = 0;
			app_bk_trace(0x02 + pt, len, 0, 0);

			state_q = get_que(&con_que, &buf, &len);

			app_bk_trace(0x02 + pt, len + ((state_q&0xFF)<<16), 0, 1);

			//printf("con: get que buf. %d\r\n", len);

			app_bk_trace(0x03 + pt, len, 0, 0);
			if(state_q && (len > 0) && (len+4 < sizeof(buf)))
			{
				buf[len++] = 0;
				buf[len++] = 0;
				buf[len++] = 0;
				c_puts_wr(buf);
			}
			app_bk_trace(0x03 + pt, len, 0, 1);

			app_bk_trace(0x04 + pt, len, 0, 0);
			clr_que(&con_que);
			app_bk_trace(0x04 + pt, len, 0, 1);
		}

		app_bk_trace(0x05 + pt, len, 0, 0);
*/
		if(ev < 0)
		{
			continue;
		}

		if(ev > 0)
		{
			if( FD_ISSET(con_wr_sock, &rfds) )
			{
				do{
					rxlen = recvfrom(con_wr_sock, buf, sizeof(buf), 0, NULL, NULL);
					if(rxlen > 0)
					{
						buf[rxlen++] = 0;
						c_puts_wr((char*)buf);

						get_que(&con_que, buf, &len);
						clr_que(&con_que);
					}
				}while(rxlen>0);

				while(get_que(&con_que, buf, &len))
				{
					clr_que(&con_que);
				}

				if(errno)
				{
					if(errno != EIDRM && errno != EINTR)
					{
						printf("con: sock read error. '%s'", strerror(errno));
						con_loop = 0; //close console
						break;
					}
				}
			}
		}
		app_bk_trace(0x05 + pt, len, 0, 1);

		app_bk_trace(0x01 + pt, ev, 0, 1);
	}

	app_trace(TRACE_INFO, "consol task_wr: exit");

	return NULL;
}

void *consol_task(void *ptr)
{
	socklen_t clilen;
	struct sockaddr_in cliaddr;

	app_trace(TRACE_INFO, "consol task start. pid %i", get_ptid());

	char proc_str[64];
	sprintf(proc_str, "[%i] consol task", get_ptid());
	add_proc_info(proc_str);

	app_bk_pid(get_ptid(), PID_TYPE_CON, 0, 0);

	con_loop = 1;

reinit_sock:
	if(!con_loop) goto con_task_exit;

	con_socket = getServerSocket(9977);

	if(con_socket < 0)
	{
		return NULL;
	}

	if(listen(con_socket, 1) < 0)
	{
		app_trace(TRACE_ERR, "Consol: cannot listen on socket");
		return NULL;
	}

	cli = cli_init();
	cli_set_banner(cli, "CFGM. Command Line Interface. (C)2009-2014 Eltex Ltd.\n");
	cli_set_hostname(cli, "CFGM");

	cli_unregister_command(cli, "enable");
	cli_unregister_command(cli, "disable");
	cli_unregister_command(cli, "terminal");
	cli_unregister_command(cli, "configure");

	cli_reg_all_cmd();

	while(con_loop)
	{
		clilen = sizeof(cliaddr);

		if((con_connid=accept(con_socket, (struct sockaddr*)&cliaddr, &clilen)) < 0)
		{
			if(errno != EINTR)
			{
				app_trace(TRACE_ERR, "Consol: accept error, errno %s", strerror(errno));
			}

			close(con_socket);				/* parent closes connected socket */
			goto reinit_sock;
		}

		else
		{
			struct linger s_linger;
			s_linger.l_onoff = 1;
			s_linger.l_linger = 0;
			setsockopt(con_connid, SOL_SOCKET, SO_LINGER, &s_linger, sizeof(s_linger));

			//app_trace(TRACE_INFO, "Consol: connect from %s", inet_ntoa(cliaddr.sin_addr));
			app_trace(-1, "Consol: connect from %s", inet_ntoa(cliaddr.sin_addr));

			register_log(c_puts);

			cli_loop_ex(cli, con_connid, con_socket, -1);

			app_trace(-1, 	"Consol: close");

			unregister_log();
			ClearAllTrace();
		}
	}

con_task_exit:
	cli_done(cli);

	app_trace(TRACE_INFO, "consol task: exit");

	return NULL;
}

void consol_close(void)
{
	con_loop = 0;

	free_que(&con_que);
	ut_mutex_destroy(con_que.mutex);
	ut_free_ex(con_que.mutex);

	if(con_wr_sock >= 0) close(con_wr_sock);
	con_wr_sock = -1;

	char s_name[64];
	sprintf(s_name, "/var/run/%s", CON_SOCK_NAME);
	unlink(s_name);

	if(con_connid >= 0)
	{
		shutdown(con_connid, 2);
		close(con_connid);
		con_connid = -1;
	}

	if(con_socket >= 0)
	{
		shutdown(con_socket, 2);
		close(con_socket);
		con_socket = -1;
	}

	app_trace(TRACE_INFO, "consol close: kill consol_thread");

	/* Kill consol thread */
	if(NULL != consol_thread)
	{
		ut_thread_cancel(*consol_thread);
		ut_thread_join(consol_thread);
		ut_free_ex(consol_thread);
		consol_thread = NULL;
	}

	app_trace(TRACE_INFO, "consol close: kill consol_thread_wr");
	/* Kill consol thread */
	if(NULL != consol_thread_wr)
	{
		ut_thread_cancel(*consol_thread_wr);
		ut_thread_join(consol_thread_wr);
		ut_free_ex(consol_thread_wr);
		consol_thread_wr = NULL;
	}

	app_trace(TRACE_INFO, "consol close: end");
}

//------------------------------------------------------------------------
