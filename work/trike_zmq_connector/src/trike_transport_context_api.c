#include <stdlib.h>
#include "trike_transport_context_api.h"

trike_transport_context_t *trike_transport_new(){
    return (trike_transport_context_t*)malloc(sizeof(trike_transport_context_t));
}

void trike_transport_free(trike_transport_context_t *transport_context){free(transport_context);};
