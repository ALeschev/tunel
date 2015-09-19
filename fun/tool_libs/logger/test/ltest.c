#include <stdio.h>

#include "logger.h"

int main (void)
{
	log_crit  ("crit");
	log_error ("error");
	log_warn  ("warn");
	log_info  ("info");
	log_debug ("debug");

	return 0;
}