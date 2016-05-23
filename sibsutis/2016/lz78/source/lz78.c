#include "lz78.h"
#include "lz78_pack.h"
#include "lz78_unpack.h"

#define INPUT_FILENAME "input_file"
#define OUTPUT_FILENAME INPUT_FILENAME".lz78"

int main(int argc, char *argv[])
{
	FILE *i_fd, *o_fd;

	i_fd = fopen(INPUT_FILENAME, "r");
	if (!i_fd)
	{
		lz78_trace("failed to open '%s'", INPUT_FILENAME);
		return -1;
	}

	o_fd = fopen(OUTPUT_FILENAME, "w");
	if (!o_fd)
	{
		lz78_trace("failed to create '%s'", OUTPUT_FILENAME);
		fclose(i_fd);
		return -1;
	}

	if (lz78_pack(i_fd, o_fd))
	{
		lz78_trace("failed to pack '%s'", INPUT_FILENAME);
	}

	fclose(i_fd);
	fclose(o_fd);

	return 0;
}
