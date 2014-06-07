#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <linux/types.h>
#include <linux/kdev_t.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <asm/types.h>

#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <getopt.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#include <pthread.h>

#include "cli/libcli.h"
#include "cli/cmdUserFunc.h"
#include "cli/common.h"

#include "ut/ut_queues.h"
#include "ut/ut_port.h"
#include "ut/ut_thread.h"
#include "ut/ut_sema.h"

#include "mqueue.h"
#include "header.h"

#include "ivr_proc.h"

mqueue_t con_que;
static int con_wr_sock = -1;
#define CON_SOCK_NAME "con_ivr"

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
int con_que_size = 0;
int con_overflow_flg = 0;
int con_mode = 0;

void consol_close(void);

//void ivrSetTrace(int lvl);


int trace_enable = 0;
//------------------------------------------------------------

static int getServerSocket(int port)
{
	int i, listenSocket;
	struct sockaddr_in listenSockaddr;

	if((listenSocket=socket(PF_INET,SOCK_STREAM,0))<0)
	{
		app_trace(TRACE_ERR, "Consol: cannot open socket");
		return -1;
	}

	i = 1;
	setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(int));
	//setsockopt(listenSocket, SOL_SOCKET, SO_REUSEPORT, &i, sizeof(int));

	memset(&listenSockaddr, 0, sizeof(listenSockaddr));

	listenSockaddr.sin_family = PF_INET;
	listenSockaddr.sin_port = htons(port);
	listenSockaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	if(bind(listenSocket,(struct sockaddr*)&listenSockaddr, sizeof(listenSockaddr)) < 0)
	{
		app_trace(TRACE_ERR, "Consol: cannot bind socket");
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
}

#ifdef _CONSOLE_EX_
void c_puts_que(char *str)
{
#ifdef MEMDBG
	return c_puts(str);
#else	

	char *buf;
	int pid = get_ptid();

	if(pid == consol_pid)
	{
		c_puts(str);
		return;
	}

	if(!con_que.state)
	{
		app_pause(1000);
		return;
	}

	if(mqueue_can_put(&con_que) < 0 || con_overflow_flg)
	{
		if(!con_overflow_flg)
		{
			c_puts(" >>>>>>>>>>>>>  [CONSOLE] queue overflow!\n");
			printf(" >>>>>>>>>>>>>  [CONSOLE] queue overflow!\n");
		}
		con_overflow_flg++;

		return;
	}

	if(NULL == (buf = (char *)ut_malloc_ex(strlen(str)+1)))
	{
		return;
	}

	strcpy(buf, str);

	if(mqueue_put(&con_que, (void*)buf, 0))
	{
		ut_free_ex(buf);
	}
#endif
}
#endif

void c_puts_wr(char *str)
{
    cli_print(cli, str);
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
	CMD_TEST
} cmd_enum;


t_trace_f trace_f[] = {
	{"off",    CMD_OFF,   0},
	{"start",  CMD_START, 0},
	{"stop",   CMD_STOP,  0},

	{"test",    CMD_TEST,  2},
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

	{"", -1}
};

void ClearAllTrace(void)
{
//	int idx;

	StopTrace();
	ivrSetTrace(0);
// #ifdef _USE_RADIUS_
// 	SetTraceRADIUS(0);
// 	SetTraceRADIUSchain(0);
// #endif

// 	mspcSetTrace(0);
// 	for(idx=0; idx<MSP_DEV_COUNT; idx++)
// 	{
// 		SetMspTrace(idx, 0);
// 	}
// #ifdef USE_ERLANG
// 	SetTraceErlL1(0);
// 	SetTraceErlL3(0);
// #endif
// 	SetTraceNet(0);
// 	SetTraceSync(0);

// 	SetTraceSNMP(0);
// 	SetTraceCfg(0);

// 	for(idx=0; idx<MAX_FRAMER; idx++)
// 	{
// 		SetTraceStream(idx, 0);
// 	}

// 	SetTraceNP(0);
// 	SetTraceAlarm(0);
// #ifdef SORM
// 	SetTraceSorm(0);
// 	SendTraceAlertToSorm(0, 0);
// #endif

// #ifdef _USE_M2UA_
// 	SetTraceM2UA(0);
//     SetTraceIUA(0);
// #endif


// 	v5dbg_con = 0;

}

void trace(char *param)
{
	char *p, str[2048];
	t_trace_f *ptf;
	int r, val=0, val2 = 0, idx = 0, lvl = 0;
	char onoff[20] = {0};

	while(*param && (*param <= ' ')) param++;

	if(!*param)
	{
		c_puts("trace point arg error\n");

		c_puts(	"     Using trace:\n"
			);

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

		c_puts(	"     Using trace:\n"
			);

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

	sprintf(str, "onoff %s, idx %d, val2 %d, lvl %d\n", onoff, idx, val2, lvl);
	c_puts (str);

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

		case CMD_IVR:
		val = ivrSetTrace(idx ? idx : val);
		sprintf(str, "set ivr-trace. %s (%d)", val ? "on":"off", val);
		c_puts(str);
		return;

	 //   case CMD_PEER:
		// val = SetTracePeer(idx ? idx : val);
		// sprintf(str, "set peer-trace. %s (%d)", val ? "on":"off", val);
		// c_puts(str);
		// return;
	}
}

// /***************************************************************************/

// 	COMMAND_HANDLER(hwreboot)
// 		app_trace(TRACE_INFO, "System will now reboot...");
// 		app_pause(500);
// 		system("reboot");
// 	END_HANDLER(hwreboot)
// /***************************************************************************/

// /***************************************************************************/
	V5_COMMAND_HANDLER(trace)
		trace(param);
	END_HANDLER(trace)
// /***************************************************************************/
	// COMMAND_HANDLER(stop)

	// 	app_trace(TRACE_INFO, "command 'stop'");

	// 	app_snd_msg(MSG_SHUTDOWN, 0, NULL);

	// 	ClearAllTrace();

	// 	unregister_log();

	// 	con_loop = 0;
	// 	if(con_connid >= 0)
	// 	{
	// 		shutdown(con_connid, 2);
	// 		close(con_connid);
	// 		con_connid = -1;
	// 	}
	// END_HANDLER(stop)

/***************************************************************************/
	COMMAND_HANDLER(regcon)
    	register_log(c_puts);
	END_HANDLER(regcon)
/***************************************************************************/
	COMMAND_HANDLER(unregcon)
    	unregister_log();
	END_HANDLER(unregcon)
/***************************************************************************/

void cli_reg_cmd(char *command, int (*callback)(struct cli_def *, char *, char **, int), char *help)
{
	if(cli != NULL)
		cli_register_command(cli, NULL, command, callback, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, help);
}

void cli_reg_all_cmd(void)
{
	//cli_reg_cmd("sys", 		cmd_sys, 	"Show system info");

	//cli_reg_cmd("show", 		cmd_show, 	"Show objects info");

	cli_reg_cmd("trace", 		cmd_trace, 	"Trace func");
	cli_reg_cmd("regcon", 		cmd_regcon, 	"Show traces in console");
	cli_reg_cmd("unregcon", 	cmd_unregcon, 	"Show traces in COM-port");
	//cli_reg_cmd("stop", 		cmd_stop, 		"stop application");
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
	app_trace(TRACE_INFO, "console: init");

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

void consol_que_proc(int fast)
{
	char *log;
	if(fast)
	{
		log = (char *)mqueue_tryget(&con_que);
	}else{
		log = (char *)mqueue_get(&con_que);
	}

	if(log)
	{
		//if(con_mode) printf(log);
		//else
			c_puts(log);

		ut_free_ex(log);
	}
}

void *consol_task_wr(void *ptr)
{
	ut_thread_setname_np("ivr-consolex");

	app_trace(TRACE_INFO, "consol task_wr: start. pid %i", get_ptid());

	int i;
	char proc_str[64];

	sprintf(proc_str, "[%i] consol-wr task", get_ptid());
	c_puts(proc_str);

	con_loop = 1;
	con_que.state = 0;

	while(con_loop)
	{
		consol_que_proc(0);

		if(con_overflow_flg)
		{
			for(i = 0; i < con_que_size; i++)
			/*while(mqueue_peek(&con_que))*/
			{
				consol_que_proc(1);
			}
			sprintf(proc_str, " >>>>>>>>>>>>>  [CONSOLE] cleared\r\n");
			c_puts(proc_str);
			printf(proc_str);
			/*app_pause(5*con_overflow_flg);*/
			//app_pause(100);
			con_overflow_flg = 0;
		}
	}

	app_trace(TRACE_INFO, "consol task_wr: exit");

	return NULL;
}

void *consol_task(void *ptr)
{
	socklen_t clilen;
	struct sockaddr_in cliaddr/*, servaddr*/;
	/*pid_t childpid;*/

	app_trace(TRACE_INFO, "consol task start. pid %i", get_ptid());

	con_loop = 1;

reinit_sock:
	if(!con_loop) goto con_task_exit;

	con_socket = getServerSocket(9981);

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
	cli_set_banner(cli, "IVR. Command Line Interface. (C)2009-2014 Eltex Ltd.\n");
	cli_set_hostname(cli, "IVR");

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

	mqueue_close (&con_que);

	app_trace(TRACE_INFO, "consol close: end");
}

//------------------------------------------------------------------------