
#define REVISION    2    /* board revision */
#define VERSION     12   /* software version */
#define SUBVERSION  1    /* software sub-version */

#define LOG_EMERG       0       /* system is unusable */
#define LOG_ALERT       1       /* action must be taken immediately */
#define LOG_CRIT        2       /* critical conditions */
#define LOG_ERR         3       /* error conditions */
#define LOG_WARNING     4       /* warning conditions */
#define LOG_NOTICE      5       /* normal but significant condition */
#define LOG_INFO        6       /* informational */
#define LOG_DEBUG       7       /* debug-level messages */

#define TRACE_ERR		LOG_ERR
#define	TRACE_WARN		LOG_WARNING
#define TRACE_INFO		LOG_INFO
#define TRACE_DEBUG		LOG_DEBUG

#define TRACE_NOTIME		0xA

#define TRACE_CON2		8

#define	TRACE_OFF		0

#define	TMG16_BOARD_NAME	"smg1016"
#define	TMG16_BOARD_MAJOR	231

#define SMG1016M_BOARD_NAME    "smg1016m"
#define SMG1016M_BOARD_MAJOR   231

#define WDOG_MODE_APP	0
#define WDOG_MODE_DRV	1

#define WDIOC_KEEPALIVE		_IOW(TMG16_BOARD_MAJOR,  0xFF, int)

#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC         1
#endif

#define WDMOD          12

#define MAX_MODULES     10

typedef struct {
	uint16_t Module_ID;

	uint32_t regtime;
	uint32_t lastping;

	uint8_t exist;
	struct stModuleInfo * next;
}   __attribute__ ((packed)) stModuleInfo;
