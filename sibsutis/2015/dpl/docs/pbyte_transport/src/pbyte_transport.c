#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <stdarg.h>
#include <sys/time.h>

#include "pbyte_transport.h"
#include "pbyte_rserver.h"
#include "pbyte_dclient.h"
#include "pbyte_log.h"

/*----------------------------------------------------------------------------*/

int pb_transport_start(pbyte_transport_t *transport, pb_params_t *transport_params)
{
	int rc = -1;

	if (!transport || !transport_params)
		return -99;

	if (transport_params->mode == ePBYTE_SERVER)
	{
		transport->mode = ePBYTE_SERVER;
		rc = rserver_start((pb_rserver_t **)&transport->choise.rserver, transport_params);
	} else
	if (transport_params->mode == ePBYTE_CLIENT)
	{
		transport->mode = ePBYTE_CLIENT;
		rc = dclient_start((pb_dclient_t **)&transport->choise.dclient, transport_params);
	} else {
		return -98;
	}

	return rc;
}

/*----------------------------------------------------------------------------*/

int pb_transport_stop(pbyte_transport_t *transport)
{
	int rc = -1;

	if (!transport)
		return -99;

	if (transport->mode == ePBYTE_SERVER)
	{
		rc = rserver_stop((pb_rserver_t *)transport->choise.rserver);
	} else
	if (transport->mode == ePBYTE_CLIENT)
	{
		rc = dclient_stop((pb_dclient_t *)transport->choise.dclient);
	} else {
		return -98;
	}

	return rc;
}

/*----------------------------------------------------------------------------*/

int pb_transport_send (pbyte_transport_t *transport,
                       identity_t *identity,
                       dialog_t *dialog,
                       char *data, int size)
{
	int rc = -1;

	if (!transport || !data || (size <= 0))
		return -99;

	if (transport->mode == ePBYTE_SERVER)
	{
		rc = rserver_send((pb_rserver_t *)transport->choise.rserver, identity, data, size);
	} else
	if (transport->mode == ePBYTE_CLIENT)
	{
		rc = dclient_send((pb_dclient_t *)transport->choise.dclient, dialog, data, size);
	} else {
		return -98;
	}

	return rc;
}

/*----------------------------------------------------------------------------*/
