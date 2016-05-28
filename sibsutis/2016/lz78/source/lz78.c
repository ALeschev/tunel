#include <getopt.h>

#include "lz78.h"
#include "lz78_pack_unpack.h"


#define LZ78_ENCODE_EXP ".lz78_enc"
#define LZ78_DECODE_EXP ".lz78_dec"

static void get_encoding_profit(const char *input, const char *output)
{
	FILE *i_fd, *o_fd;
	long input_file_size, output_file_size;
	float pack_profit;

	i_fd = fopen(input, "r");
	if (!i_fd)
	{
		lz78_trace("failed to open '%s'", input);
		return;
	}

	o_fd = fopen(output, "r");
	if (!o_fd)
	{
		lz78_trace("failed to open '%s'", output);
		fclose(i_fd);
		return;
	}

	fseek(i_fd, 0L, SEEK_END);
	input_file_size = ftell(i_fd);

	fseek(o_fd, 0L, SEEK_END);
	output_file_size = ftell(o_fd);

	pack_profit = 100 - (float)output_file_size/(float)input_file_size * 100;

	lz78_trace("input file size: %ld", input_file_size);
	lz78_trace("output file size: %ld", output_file_size);
	lz78_trace("pack profit: %.2f", pack_profit);

	fclose(i_fd);
	fclose(o_fd);
}

int main(int argc, char *argv[])
{
	int c, encoding = 0, decoding = 0;
	char *input_filename = NULL;
	char output_filename[512] = {0};

	if(argc == 1)
	{
		lz78_trace("Options:");
		lz78_trace("-e <input filename> - encoding by lz78");
		lz78_trace("-d <input filename> - decoding by lz78");

		return -1;
	}

	while((c = getopt(argc, argv, "e:d:")) != -1)
	{
		switch(c)
		{
			case 'e': encoding = 1; input_filename = optarg; break;
			case 'd': decoding = 1; input_filename = optarg; break;

			default:
				return -1;
		}
	}

	if (!encoding && !decoding)
	{
		lz78_trace("Encoding or decoding action is not present");
		return -1;
	}

	if (!input_filename)
	{
		lz78_trace("Input filename is not present");
		return -1;
	}

	if (encoding)
	{
		snprintf(output_filename, sizeof(output_filename),
		         "%s"LZ78_ENCODE_EXP, input_filename);

		if (!lz78_pack(input_filename, output_filename))
			get_encoding_profit(input_filename, output_filename);
	}

	if (decoding)
	{
		snprintf(output_filename, sizeof(output_filename),
		         "%s"LZ78_DECODE_EXP, input_filename);

		lz78_unpack(input_filename, output_filename);
	}

	return 0;
}
