#include "encoding.h"
#include "parity_matrix.h"
#include "bit_array.h"
#include "frag_sesh.h"
#include "constants.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>		// memset(), memcpy()
#include <assert.h>		// assert()
#include <stdlib.h>    //  malloc(), free()

/* **************************************************************************************
*										GLOBAL VARS
*  *************************************************************************************/
extern frag_sesh_t fs;
extern uint8_t* storage;

extern bit_array_t C;


/* **************************************************************************************
*										PRIVATE VARS & FUNCS
*  *************************************************************************************/



/* **************************************************************************************
*										PUBLIC FUNCTIONS
*  *************************************************************************************/
void encode(uint8_t* data_in, uint8_t* fec_data_out) {
	assert(data_in != NULL);
	assert(fec_data_out != NULL);

	uint16_t n,i;
	uint8_t* encoded_fragment = (uint8_t*)malloc(sizeof(uint8_t) * fs.frag_size);
	TRACE("encoding\n\r");

	if (get_bit_array(&C, fs.M) && encoded_fragment != NULL) {
		for (n = 0; n < fs.N; n++) {
			// fill computed fragment with zeros initially
			memset(encoded_fragment, 0x00, sizeof(uint8_t) * fs.frag_size);
			// get the parity matrix row for fragment n
			get_matrix_line(n, fs.M, &C, fs.parity_fraction);
			print_bit_array(&C);

			for (i = 0; i < fs.M; i++) {
				// if fragment 'i' is used to build up this encoded fragment...
				if (get_bit(&C, i)) {
					//printf("gb: %u n: %u ", i,n);
					// then "add" it to the mix with an XOR
					bitwise_array_XOR(encoded_fragment, \
						data_in + sizeof(uint8_t) * i * fs.frag_size, fs.frag_size);
				}
			}

			// transfer to output data for fragment location 'n'
			memcpy(fec_data_out + sizeof(uint8_t) * n * fs.frag_size, \
				encoded_fragment, sizeof(uint8_t) * fs.frag_size);
		}
	}
	
	// release mallocated memory
	free(encoded_fragment);

	return;
}


void bitwise_array_XOR(uint8_t* A, uint8_t* B, uint16_t num_bytes) {
	assert(A != NULL);
	assert(B != NULL);

	// do a byte by byte XOR --->  A = A ^ B
	uint16_t i;
	for (i = 0; i < num_bytes; i++) {
		*(A + sizeof(uint8_t) * i) = *(A + sizeof(uint8_t) * i) ^ *(B + sizeof(uint8_t) * i);
	}

	return;
}
	