#include "check_arguments.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "frag_sesh.h"
#include "constants.h"
#include <assert.h>



/* **************************************************************************************
*										GLOBAL VARS
*  *************************************************************************************/
extern frag_sesh_t fs;
extern uint8_t packet_delivery_rate;
extern int VERBOSE;
extern int QUIET;



/* **************************************************************************************
*											MAIN
*  *************************************************************************************/
int check_arguments(int argc, char* argv[]) {
	int ret = FAIL;
	int i;
	char verbose[] = "";

	// check correct number of arguments
	if (argc != 7 &&  argc!= 8)
	{
		printf("usage:\n");
		printf("patch_frag_tx.exe  [patch_file_length] \
[fragment size] [coding rate] [packet delivery rate %%] \
 [passes] [parity fraction] <verbose>\n");
		printf("\n\rwhere:\n\r \tpatch_file_length\tin bytes\n\r");
		printf("\tfragment_size\t\tbetween 8 and 48 bytes\n\r");
		printf("\tcoding_rate\t\tbetween 1.0 and 2.0 times\n\r");
		printf("\tpacket_delivery_rate\tbetween 33 and 100%%\n\r");
		printf("\tpasses\t\t\tnumber of passes through the data {1,2,3}\n\r");
		printf("\tparity fraction\t\t\tfraction of matrix line to contain fragments {2 to 8} \n\r");
		printf("\t<verbose>\t\tv = lots of debug output\n\r");
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
	if (1 != sscanf_s(argv[4], "%hhu", &packet_delivery_rate))
	{
		printf("could not parse packet delivery rate\n");
		return ret;
	}

	// check that passes format is OK
	if (1 != sscanf_s(argv[5], "%hhu", &fs.passes))
	{
		printf("could not parse number of passes\n");
		return ret;
	}

	// check that parity fraction format is OK
	if (1 != sscanf_s(argv[6], "%hhu", &fs.parity_fraction))
	{
		printf("could not parse parity fraction\n");
		return ret;
	}
	 
	// check that Verbose command format is OK
	if ( (argc == 8) && (1 != sscanf_s(argv[7], "%s", verbose)))
	{
		printf("could not parse optional verbose switch (v)\n");
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

	// check limits on number of passes
	if ((fs.passes < 1) | (fs.passes > 3)) {
		printf("number of passes out of limits [1 to 3]\n\r");
	}

	// Calculate M, N and padding
	fs.M = fs.data_length / (uint16_t)fs.frag_size + (fs.data_length % fs.frag_size != 0);
	fs.N = (uint16_t)((float)fs.M * fs.coding_rate);
	fs.padding_bytes = fs.M * fs.frag_size - fs.data_length;
	if (fs.padding_bytes == fs.frag_size) fs.padding_bytes = 0;
	fs.pdr = packet_delivery_rate;


	// check limits on M and N
	assert(fs.M <= 512);
	assert(fs.N <= 1024);

	print_session();

	if (strcmp(verbose,"v")==0) {
		ret = VERBOSE;
	}
	else {
		ret = QUIET;
	}
	return ret;
}