/*
* LoRaWAN Fragmentation Encodder/Decoder Demonstration for Embedded with low memory usage
*
* O Bailey Nov 2022
*
* This demonstrates  the fragmentation encoding (server side) and decoding (device side)
* for a LoRaWAN file transfer (typically a new firmware  or firmware patch)
*
* The original data is a sawtooth pattern counting upwards between 0 to 255.
*
*/
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>	
#include <stdlib.h>
#include "bit_array.h"
#include "parity_matrix.h"
#include "storage.h"
#include "channel.h"
#include "encoding.h"
#include "decoding.h"
#include "frag_sesh.h"
#include "constants.h"

const int FAIL = -1;
const int SUCCESS = 0;



int main(int argc, char* argv[]) {
	int		  i;
	int		  ret = FAIL;
	uint16_t  data_length = 0;
	uint8_t   frag_size = 0;
	float     coding_rate = 0;
	uint8_t	  packet_delivery_rate = 0;
	uint8_t*  patch_data = NULL;
	uint8_t*  frag = NULL;
	uint8_t*  FEC_data = NULL;
	uint16_t  packet_num;
	frag_sesh_t frag_sesh;
	frag_sesh_t* fs = &frag_sesh;


	// check correct number of arguments
	if (argc != 5)
	{
		printf("usage:\n");
		printf("patch_frag_tx.exe  [patch_file_length] \
					 [fragment size] [coding rate] [packet delivery rate %%]\n");
		goto end;
	}
	else { 
		for (i = 0; i < argc; i++) {
			printf("%s ", argv[i]);
		}
		printf("\n\r");
	}

	
	// check that patch file length format is OK
	if (1 != sscanf_s(argv[1], "%hu", &data_length))
	{
		printf("could not parse patch file size\n");
		goto end;
	}

	// check that fragment size format is OK
	if (1 != sscanf_s(argv[2], "%hhu", &frag_size))
	{
		printf("could not parse fragment size\n");
		goto end;
	}

	// check that Coding Rate format is OK
	if (1 != sscanf_s(argv[3], "%f", &coding_rate))
	{
		printf("could not parse coding rate\n");
		goto end;
	}

	// check that PDR format is OK
	if (1 != sscanf_s(argv[4], "%hhu", &packet_delivery_rate))
	{
		printf("could not parse packet delivery rate\n");
		goto end;
	}

	// check limits on patch file size
	if ((data_length < 256) | (data_length > 24576)) {
		printf("patch file size out of limits [512 to 24576 bytes]\n\r");
		goto end;
	}

	// check limits on fragment size
	if ((frag_size < 8) | (frag_size > 48)) {
		printf("fragment size out of limits [8 to 48 bytes ]\n\r");
		goto end;
	}

	// check limits on coding rate
	if ((coding_rate < 1.0) | (coding_rate > 2.0)) {
		printf("coding rate out of limits [1.0to 2.0 ]\n\r");
		goto end;
	}

	// check limits of PDR
	if ((packet_delivery_rate < 33) | (packet_delivery_rate > 100)) {
		printf("packet delivery rate out of limits [33 to 100 %%]\n\r");
	}

	// Calculate M, N and padding
	const uint16_t  M = data_length / (uint16_t)frag_size + (data_length % frag_size != 0);
	const uint16_t  N = (uint16_t)((float)M * coding_rate);
	uint8_t   num_padding_bytes = M * frag_size - data_length;
	if (num_padding_bytes == frag_size) {
		num_padding_bytes = 0;
	}


	// check limits on M and N
	assert(M <= 512);
	assert(N <= 1024);

	printf("M: %u N: %u Padding: %u Data length: %u Fragment Size: %u\n\r", \
		M,N, num_padding_bytes, data_length, frag_size);

	frag_sesh.data_length = data_length;
	frag_sesh.frag_size = frag_size;
	frag_sesh.M = M;
	frag_sesh.N = N;
	frag_sesh.padding_bytes = num_padding_bytes;
	frag_sesh.coding_rate = coding_rate;

	// create patch data (with padding)
	patch_data = (uint8_t*)malloc(sizeof(uint8_t) * M * frag_size);
	if (check_for_null((void*)patch_data, &ret)) goto end;
	
	// initially set the patch data to all zeros
	memset(patch_data, 0x00, (size_t)(M * frag_size));

	// create the sequence data for the raw "patch file"
	for (i = 0; i < data_length; i++) {
		*(patch_data + i) = i & 0xFF;
	}

	// allocate memory for encoded output data
	FEC_data = (uint8_t*)malloc(sizeof(uint8_t) * N * frag_size);
	if (check_for_null((void*)FEC_data, &ret)) goto end;

	memset(FEC_data, 0x00, (size_t)(N* frag_size));

	// encode the patch data, expanding it with redundancy in (N-M) extra fragments
	encode(patch_data, FEC_data, N, M, frag_size);

	// memory to store a received fragment for processing
	frag = (uint8_t*) malloc(sizeof(uint8_t) * frag_size);
	if (check_for_null((void*)frag, &ret)) goto end;

	// create the storage space for the received  and decoded fragments
	uint8_t* storage_ptr = create_storage(M, frag_size);

	if (init_decoder(storage_ptr, frag_size, M, N) == FAIL) {
		ret = FAIL;
		goto end;
	}
	
	// receive the messages (with some getting lost on the way) and decode one-by-one
	do {
		// simulate a lossy LoRa radio link
		packet_num = receive_next_fragment(frag, FEC_data, N, frag_size, packet_delivery_rate);

		// and pass each received message to the decoder as they arrive
		decode_fragment(frag, packet_num);
		

	} while (packet_num < (N-1));

	
	ret = SUCCESS;

end:
	// free up allocated heap memory (it's OK to free a NULL pointer too if it wasn't used...)
	free_decoder_memory();
	free(patch_data);
	free(frag);
	free(FEC_data);


	// say goodbye
	printf("\n\rexiting: %i\n\r", ret);
	
	return ret;
}





/*	
// test bit_array_t
	bit_array_t* b_ptr1 = malloc(sizeof(bit_array_t));

	if (get_bit_array(b_ptr1, M)) {
		print_bit_array(b_ptr1);
		set_bit(b_ptr1, 3);
		set_bit(b_ptr1, 7);
		set_bit(b_ptr1, 8);
		set_bit(b_ptr1, M - 1);
		printf("get_bit: %u\n\r",get_bit(b_ptr1,3));
		print_bit_array(b_ptr1);
		printf("No. of ones: %u\n\r", number_of_ones(b_ptr1));
		clear_all_bits(b_ptr1);
		print_bit_array(b_ptr1);
		delete_bit_array(b_ptr1);
	}


	// test C
	bit_array_t* C_ptr = malloc(sizeof(bit_array_t));
	if (get_bit_array(C_ptr, M)) {
		get_matrix_line(0, M, C_ptr);
		print_bit_array(C_ptr);

		get_matrix_line(M - 1, M, C_ptr);
		print_bit_array(C_ptr);

		get_matrix_line(M, M, C_ptr);
		print_bit_array(C_ptr);
		printf("No. of ones: %u\n\r", number_of_ones(C_ptr));

		get_matrix_line(M + 10, M, C_ptr);
		print_bit_array(C_ptr);
		printf("No. of ones: %u\n\r", number_of_ones(C_ptr));
		delete_bit_array(C_ptr);

	}
*/





