#include "encoding.h"
#include "parity_matrix.h"
#include "bit_array.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>		// memset(), memcpy()
#include <assert.h>		// assert()
#include <stdlib.h>    //  malloc(), free()


void encode(uint8_t* data_in, uint8_t* fec_data_out, uint16_t N, uint16_t M, uint8_t frag_size) {
	assert(data_in != NULL);
	assert(fec_data_out != NULL);

	uint16_t n,i;
	bit_array_t* C = malloc(sizeof(bit_array_t));
	uint8_t* encoded_fragment = (uint8_t*)malloc(sizeof(uint8_t) * frag_size);
	

	if (get_bit_array(C, M) && encoded_fragment != NULL) {
		for (n = 0; n < N; n++) {
			// fill computed fragment with zeros initially
			memset(encoded_fragment, 0x00, sizeof(uint8_t) * frag_size);
			// get the parity matrix row for fragment n
			get_matrix_line(n, M, C);
			print_bit_array(C);

			for (i = 0; i < M; i++) {
				// if fragment 'i' is used to build up this encoded fragment...
				if (get_bit(C, i)) {
					//printf("gb: %u n: %u ", i,n);
					// then "add" it to the mix with an XOR
					bitwise_array_XOR(encoded_fragment, data_in + sizeof(uint8_t) * i * frag_size, frag_size);
				}
			}

			// transfer to output data for fragment location 'n'
			memcpy(fec_data_out + sizeof(uint8_t) * n * frag_size, encoded_fragment, sizeof(uint8_t) * frag_size);
		}
	}
	
	// release mallocated memory
	//free(C);
	delete_bit_array(C);
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
	