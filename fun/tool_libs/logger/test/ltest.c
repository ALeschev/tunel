#include <stdio.h>

#include "logger.h"

void print_trace (void)
{
	log_crit  ("crit");
	log_error ("error");
	log_warn  ("warn");
	log_info  ("info");
	log_debug ("debug");
}

int main (void)
{
	log_set_prio (eLOG_FULL);
	print_trace ();
	log_set_prio (eLOG_CUT);
	print_trace ();
	log_set_prio (eLOG_INFO);
	print_trace ();
	log_set_prio (eLOG_WARN);
	print_trace ();
	log_set_prio (eLOG_ERR);
	print_trace ();
	log_set_prio (eLOG_CRIT);
	print_trace ();

	return 0;
}