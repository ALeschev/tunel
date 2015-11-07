// #include "rtpsw.h"

#define STAT_OVERFLOW_PADDING 1000
#define STAT_UPDATE_PERIOD 1000

#if BITS_PER_LONG == 64
#define STAT_OVERFLOW(stat) atomic_long_read(stat) >= (INT_MAX - STAT_OVERFLOW_PADDING)
#else
#define STAT_OVERFLOW(stat) atomic_long_read(stat) >= (LONG_MAX - STAT_OVERFLOW_PADDING)
#endif

#define stat_read(stat) atomic_long_read(stat)
#define stat_set(stat, val) atomic_long_set(stat, val)
#define stat_inc(stat) atomic_long_inc(stat)

enum stat_pkt_count
{
	eSTAT_PKT_IN           = 0,
	eSTAT_PKT_DROP         = 1,
	eSTAT_PKT_OUT          = 2,

	eSTAT_PKT_IN_SEC       = 3,
	eSTAT_PKT_OUT_SEC      = 4,

	eSTAT_PKT_IN_SEC_MAX   = 5,
	eSTAT_PKT_OUT_SEC_MAX  = 6,

	eSTAT_PREV_PKT_IN_VAL  = 7,
	eSTAT_PREV_PKT_OUT_VAL = 8,

	eSTAT_PKT_MAX
};

static struct timer_list statistics_timer;
static atomic_long_t statistic_pkt[eSTAT_PKT_MAX];

static inline void st_compute_pkt_in_sec(void)
{
	long pkt_in_sec;
	long pkt_in = stat_read(&statistic_pkt[eSTAT_PKT_IN]);
	long prev_pkt_in_val = stat_read(&statistic_pkt[eSTAT_PREV_PKT_IN_VAL]);
	long pkt_in_sec_max = stat_read(&statistic_pkt[eSTAT_PKT_IN_SEC_MAX]);

	pkt_in_sec = pkt_in - prev_pkt_in_val;

	stat_set(&statistic_pkt[eSTAT_PKT_IN_SEC], pkt_in_sec);

	if (pkt_in_sec > pkt_in_sec_max)
		stat_set(&statistic_pkt[eSTAT_PKT_IN_SEC_MAX], pkt_in_sec);

	stat_set(&statistic_pkt[eSTAT_PREV_PKT_IN_VAL], pkt_in);
}

static inline void st_compute_pkt_out_sec(void)
{
	long pkt_out_sec;
	long pkt_out = stat_read(&statistic_pkt[eSTAT_PKT_OUT]);
	long prev_pkt_out_val = stat_read(&statistic_pkt[eSTAT_PREV_PKT_OUT_VAL]);
	long pkt_out_sec_max = stat_read(&statistic_pkt[eSTAT_PKT_OUT_SEC_MAX]);

	pkt_out_sec = pkt_out - prev_pkt_out_val;

	stat_set(&statistic_pkt[eSTAT_PKT_OUT_SEC], pkt_out_sec);

	if (pkt_out_sec > pkt_out_sec_max)
		stat_set(&statistic_pkt[eSTAT_PKT_OUT_SEC_MAX], pkt_out_sec);

	stat_set(&statistic_pkt[eSTAT_PREV_PKT_OUT_VAL], pkt_out);
}

static void queue_statistics_timer(unsigned long data)
{
	st_compute_pkt_in_sec();
	st_compute_pkt_out_sec();

	statistics_timer.expires = jiffies + msecs_to_jiffies(STAT_UPDATE_PERIOD);
	add_timer(&statistics_timer);
}

static inline void st_init(void)
{
	int i;

	for (i = 0; i < eSTAT_PKT_MAX; i++)
	{
		stat_set(&statistic_pkt[i], 0);
	}

	init_timer(&statistics_timer);
	statistics_timer.function = &queue_statistics_timer;
	statistics_timer.data = 0;
	statistics_timer.expires = jiffies + msecs_to_jiffies(STAT_UPDATE_PERIOD);
	add_timer(&statistics_timer);
}

static inline void st_deinit(void)
{
	del_timer(&statistics_timer);
}

static inline void st_pkt_inc_uni(enum stat_pkt_count stat)
{
	atomic_long_t *pkt_stat = &statistic_pkt[stat];

	if (STAT_OVERFLOW(pkt_stat))
	{
		stat_set(pkt_stat, 0);
	}
	else
	{
		stat_inc(pkt_stat);
	}
}

static inline void st_pkt_in_inc(void)
{
	st_pkt_inc_uni(eSTAT_PKT_IN);
}

static inline void st_pkt_out_inc(void)
{
	st_pkt_inc_uni(eSTAT_PKT_OUT);
}

static inline void st_pkt_drop_inc(void)
{
	st_pkt_inc_uni(eSTAT_PKT_DROP);
}

static inline ssize_t st_pkt_print_to_buff (char *buff, ssize_t len)
{
	ssize_t result = 0;

	result = snprintf(buff, len, "RX: %lu pkt/sec %lu (max %lu)\n"
	                             "TX: %lu pkt/sec %lu (max %lu)\n"
	                             "dropped: %lu\n",
	                             stat_read(&statistic_pkt[eSTAT_PKT_IN]),
	                             stat_read(&statistic_pkt[eSTAT_PKT_IN_SEC]),
	                             stat_read(&statistic_pkt[eSTAT_PKT_IN_SEC_MAX]),
	                             stat_read(&statistic_pkt[eSTAT_PKT_OUT]),
	                             stat_read(&statistic_pkt[eSTAT_PKT_OUT_SEC]),
	                             stat_read(&statistic_pkt[eSTAT_PKT_OUT_SEC_MAX]),
	                             stat_read(&statistic_pkt[eSTAT_PKT_DROP]));

	return result;
}