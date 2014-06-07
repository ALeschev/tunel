#ifndef IVR_H
#define IVR_H

#include "pbx_def.h"

typedef struct _IVRCallData
{
	int    CState;         /* Call state */

	unsigned long Timer;   /* per second timer */

	int    Callref;

	XPORT  owner; /*onwer service port */

	number_t calling;

	stIVRScenario scr_data;
	int scr_idx;

} tIVRCallData;

int ivr_init(void);
void ivr_close(void);

int ivrProcessEnter(XPORT xport, int ivr_idx);
void ivrProcessStop(int callref);
int ivrInDigit(XPORT xport, uint8_t digit);
void ivr_queue_put (void *element, int prio);

#endif /*IVR_H*/