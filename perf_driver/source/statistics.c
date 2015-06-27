// #include "rtpsw.h"

#define ST_OVERFLOW_PAD

atomic_long_t pkt_in;
atomic_long_t pkt_out;

static inline void st_init(void)
{
	atomic_set(&pkt_in, 0);
	atomic_set(&pkt_out, 0);
}

static inline void st_pkt_in_inc(void)
{
// #if BITS_PER_LONG == 64
// 	if (atomic_long_read(pkt_in) >= (INT_MAX - ST_OVERFLOW_PAD))
// #else
// 	if (atomic_long_read(pkt_in) >= (LONG_MAX - ST_OVERFLOW_PAD))
// #endif

	
// 	{

// 	}

	atomic_long_inc(&pkt_in);
}

static inline void st_pkt_in_out(void)
{
	atomic_long_inc(&pkt_out);
}

static inline long st_pkt_in_read(void)
{
	return atomic_long_read(&pkt_in);
}
