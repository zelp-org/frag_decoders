#include "check_arguments.h"
#include <stdint.h>
#include <stdio.h>
#include "frag_sesh.h"
#include "constants.h"
#include <assert.h>



/* **************************************************************************************
*										GLOBAL VARS
*  *************************************************************************************/
extern frag_sesh_t fs;
extern packet_delivery_rate;


/* **************************************************************************************
*											MAIN
*  *************************************************************************************/
int check_arguments(int argc, char* argv[]) {
	int ret = FAIL;
	int i;

	// check correct number of arguments
	if (argc != 5)
	{
		printf("usage:\n");
		printf("patch_frag_tx.exe  [patch_file_length] \
					 [fragment size] [coding rate] [packet delivery rate %%]\n");
		return ret;
	}
	else {
		for (i = 0; i < argc; i++) {
			printf("%s ", argv[i]);
		}
		printf("\n\r");
	}


	// check that patch file length format is OK
	if (1 != sscanf_s(argv[1], "%hu", &fs.data_length))
	{
		printf("could not parse patch file size\n");
		return ret;
	}

	// check that fragment size format is OK
	if (1 != sscanf_s(argv[2], "%hhu", &fs.frag_size))
	{
		printf("could not parse fragment size\n");
		return ret;
	}

	// check that Coding Rate format is OK
	if (1 != sscanf_s(argv[3], "%f", &fs.coding_rate))
	{
		printf("could not parse coding rate\n");
		return ret;
	}

	// check that PDR format is OK
	if (1 != sscanf_s(argv[4], "%u", &packet_delivery_rate))
	{
		printf("could not parse packet delivery rate\n");
		return ret;
	}

	// check limits on patch file size
	if ((fs.data_length < 32) | (fs.data_length > 24576)) {
		printf("patch file size out of limits [512 to 24576 bytes]\n\r");
		return ret;
	}

	// check limits on fragment size
	if ((fs.frag_size < 8) | (fs.frag_size > 48)) {
		printf("fragment size out of limits [8 to 48 bytes ]\n\r");
		return ret;
	}

	// check limits on coding rate
	if ((fs.coding_rate < 1.0) | (fs.coding_rate > 2.0)) {
		printf("coding rate out of limits [1.0to 2.0 ]\n\r");
		return ret;
	}

	// check limits of PDR
	if ((packet_delivery_rate < 33) | (packet_delivery_rate > 100)) {
		printf("packet delivery rate out of limits [33 to 100 %%]\n\r");
	}

	// Calculate M, N and padding
	fs.M = fs.data_length / (uint16_t)fs.frag_size + (fs.data_length % fs.frag_size != 0);
	fs.N = (uint16_t)((float)fs.M * fs.coding_rate);
	fs.padding_bytes = fs.M * fs.frag_size - fs.data_length;
	if (fs.padding_bytes == fs.frag_size) fs.padding_bytes = 0;


	// check limits on M and N
	assert(fs.M <= 512);
	assert(fs.N <= 1024);

	print_session(&fs);

	ret = SUCCESS;
	return ret;
}