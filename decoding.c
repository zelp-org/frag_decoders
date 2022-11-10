
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>			// malloc, free
#include <string.h>			// memset, memcpy
#include <assert.h>
#include "bit_array.h"
#include "decoding.h"
#include "encoding.h"		// bitwise_array_XOR
#include "storage.h"		// store_fragment
#include "parity_matrix.h"	// get_matrix_line
#include "constants.h"


static bit_array_t* uncoded_fragments;
static bit_array_t C;
static bit_array_t* C_ptr;
static uint8_t* A;
static uint16_t _frag_size;
static uint16_t _M;
static uint16_t _N;
static uint8_t* _storage;

static void XOR_bytes_with_bit_array(bit_array_t* X, uint8_t* Y);
static void copy_C_to_A_at_row(bit_array_t* C, uint8_t* A, uint16_t j);


int init_decoder(uint8_t* storage, uint16_t frag_size, uint16_t M, uint16_t N) {
	assert(storage != NULL);

	// fragments to be received up to packet_num=(M-1) are uncoded, so 
	// just note down if they were received in a bit array
    uncoded_fragments = (bit_array_t*)malloc(sizeof(bit_array_t));
	if (!get_bit_array(uncoded_fragments, M)) {
		return FAIL ;
	}

	// get a bit array to hold a parity matrix line C
	bit_array_t* C_ptr = &C;
	if (!get_bit_array(C_ptr, M)) {
		return FAIL;
	}



	// TEMPORARY: Use byte matrix for A (massive memory usage for large M!!!)
	A = malloc(sizeof(uint8_t) * M * M);
	if (A == NULL) return FAIL;
	memset(A, 0x00, (size_t)(M * M));

	_frag_size = frag_size;
	_M = M;
	_N = N;
	_storage = storage;

	return SUCCESS;
}

void free_decoder_memory(void) {
	free(A);
	free(C_ptr);
	free(uncoded_fragments);

	return;
}



void decode_fragment(uint8_t* frag, uint16_t packet_num) {
	assert(frag != NULL);
	uint16_t i,j;

	printf("ahem...");
	print_bit_array(C_ptr);
	printf("excuse me...");
	printf("%u\n\r", number_of_ones(C_ptr));
	// process each received packet one by one, based on its number
	if (packet_num < _M) {
		// mark this uncoded packet as received
		set_bit(uncoded_fragments, packet_num);
		// store it to the right place in the store
		store_fragment(packet_num, _frag_size, frag, _storage);
		printf("stored uncoded fragment: %u\n\r", packet_num);
	}
	else {
		// for encoded fragments with packet number > M, get its corresponding parity matrix line
		get_matrix_line(packet_num, _M, C_ptr);

		for (i = 0; i < _M; i++) {
			// for each 1 in C in position i...
			if (get_bit(C_ptr, i)) {
				// check if row i of A contains one or more 1s
				if (number_of_ones(uncoded_fragments) >= 1) {
					// if yes, XOR line A(i,) with C and store back to C
					XOR_bytes_with_bit_array(C_ptr, A + sizeof(uint8_t) * i * _M);

					// and also XOR frag with S(i) and store back to frag
					bitwise_array_XOR(frag, _storage + sizeof(uint8_t) * i * _frag_size, _frag_size);
				}
			}
		}
	}

	// now either C is all zeros, which means fragment has no useful info, so skip
	printf("here1 ");
	print_bit_array(C_ptr);
	printf("here2 ");
	if (number_of_ones(C_ptr) == 0) {
		printf("here3 ");
		return;
	}
	

	// or C is non-null so write C to row j in A, where j is the index of the
	// first non-zero element in C
	j = index_of_first_one(C_ptr);
	copy_C_to_A_at_row(C_ptr, A, j);

	// and also add the XOR-modified fragment to the S at position i
	memcpy(frag, _storage + sizeof(uint8_t) * i * _frag_size, _frag_size);

	// and repeat till all rows of A have been updated(???)
}

static void XOR_bytes_with_bit_array(bit_array_t* X, uint8_t* Y) {
	uint16_t i;
	// create bit array version of bytes in Y
	bit_array_t* Yba = (bit_array_t*)malloc(sizeof(bit_array_t));
	get_bit_array(Yba, X->numBits);

	// set bits of temp bit array according to bytes of Y
	for (i = 0; i < X->numBits; i++) {
		if (*(Y + sizeof(uint8_t) * i) > 0) {
			set_bit(Yba, i);
		}
	}
	// now XOR X with Yba as bit arrays
	bitwise_XOR(X, Yba);

	//free(Yba);
	delete_bit_array(Yba);
	return;
}

static void copy_C_to_A_at_row(bit_array_t* C, uint8_t* A, uint16_t j) {
	uint16_t i;

	for (i = 0; i < C->numBits; i++) {
		if (get_bit(C, i)) {
			*(A + sizeof(uint8_t) * i * C->numBits) = 0x01;
		}
	}

	return;
}
