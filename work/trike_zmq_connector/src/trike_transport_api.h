#ifndef _TRIKE_TRANSPORT_API_H
#define _TRIKE_TRANSPORT_API_H

#include "trike_transport_context.h"

trike_transport_context_t *trike_transport_new();

void trike_transport_free(trike_transport_context_t *transport_context);

#endif
