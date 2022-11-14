
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
#include "frag_sesh.h"

/* **************************************************************************************
*										GLOBAL VARS
*  *************************************************************************************/
extern frag_sesh_t fs;
extern uint8_t* storage;

extern bit_array_t* tally_of_rxd_frags;
extern bit_array_t C;
extern uint8_t* A;

/* **************************************************************************************
*										PRIVATE VARS & FUNCS
*  *************************************************************************************/
static void XOR_bytes_with_bit_array(bit_array_t* X, uint8_t* Y);
static void copy_C_to_A_at_row(bit_array_t* C, uint8_t* A, uint16_t j);


/* **************************************************************************************
*										PUBLIC FUNCTIONS
*  *************************************************************************************/
int init_decoder() {
	assert(storage != NULL);

	// fragments to be received up to packet_num=(M-1) are uncoded, so 
	// just note down if they were received in a bit array
    tally_of_rxd_frags = (bit_array_t*)malloc(sizeof(bit_array_t));
	if (!get_bit_array(tally_of_rxd_frags, fs.M)) {
		return FAIL ;
	}

	// get a bit array to hold a parity matrix line C
	if (!get_bit_array(&C, fs.M)) {
		return FAIL;
	}

	// TEMPORARY: Use byte matrix for A (massive memory usage for large M!!!)
	A = malloc(sizeof(uint8_t) * fs.M * fs.M);
	if (A == NULL) return FAIL;
	memset(A, 0x00, (size_t)(fs.M * fs.M));

	return SUCCESS;
}



void decode_fragment(uint8_t* frag, uint16_t packet_num) {
	assert(frag != NULL);
	uint16_t i, j;

	// not sure why I need to do this...
	packet_num = packet_num - 1;

	// process each received packet one by one, based on its number
	if (packet_num <= fs.M) {
		// mark this uncoded packet as received
		set_bit(tally_of_rxd_frags, packet_num);
		// set identity diagonal of A for this packet
		*(A + sizeof(uint8_t) * fs.M * packet_num) = 0x01;

		// store it to the right place in the store
		store_fragment(packet_num, frag);
		//printf("stored uncoded fragment: %u\n\r", packet_num);
	}
	else {
		// for encoded fragments with packet number > M, get its corresponding parity matrix line
		get_matrix_line(packet_num, fs.M, &C);
		print_bit_array(&C);

		for (i = 0; i < fs.M; i++) {
			// for each 1 in C in position i...
			if (get_bit(&C, i)) {
				// check if row i of A contains one or more 1s
				if (get_bit(tally_of_rxd_frags, i)) {
					printf(" t: %u " , i);
					
					// if yes, XOR line A(i,) with C and store back to C
					XOR_bytes_with_bit_array(&C, A + sizeof(uint8_t) * i * fs.M);

					// and also XOR frag with S(i) and store back to frag
					bitwise_array_XOR(frag, storage + sizeof(uint8_t) * i * fs.frag_size, fs.frag_size);
				}
			}
		}


		// now either C is all zeros, which means fragment has no useful info, so skip
		if (number_of_ones(&C) == 0) {
			printf("C is null - frag has no useful XORd info - discarding\n\r");
			return;
			
		}


		// or C is non-null so write C to row j in A, where j is the index of the
		// first non-zero element in C
		j = index_of_first_one(&C);
		printf(" J: %u ", j);
		print_bit_array(&C);
		printf("index of first one in C: %u\n\r", j);
		copy_C_to_A_at_row(&C, A, j);

		// and also add the XOR-modified fragment to the S at position j
		memcpy(frag, storage + sizeof(uint8_t) * j * fs.frag_size, fs.frag_size);
	}
}

void final_step() {
	uint16_t i, j;
	// starting from matrix line i = (M - 1) down to 1, fetch the(i)th line of A.
	// the line A(i) has a one at position i, and some beyond, but only zeros to the left.
	// for any 1 at position j > i, XOR S(i) with S(j) and update S(i) with the result
	for (i = (fs.M - 1); i > 0; i--) {
		for (j = i + 1; j < fs.M; j++) {
			printf("i: %u, j: %u A[i,j]: %u\n\r", i, j,*(A + sizeof(uint8_t) * i * fs.M + j) );
			if (*(A + sizeof(uint8_t) * i * fs.frag_size + i) = 0x01) {
				//bitwise_array_XOR(storage + sizeof(uint8_t) * fs.frag_size * j, \
					storage + sizeof(uint8_t) * fs.frag_size * j, fs.frag_size);
			}
		}
	}
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

static void copy_C_to_A_at_row(bit_array_t* C, uint8_t * A, uint16_t j) {
	uint16_t i;

	for (i = 0; i < C->numBits; i++) {
		if (get_bit(C, i)) {
			*(A + sizeof(uint8_t) * i * C->numBits) = 0x01;
		}
	}

	return;
}

bool is_decoded_same_as_original(uint8_t* patch_data) {
	uint16_t i = 0;
	uint8_t X, Y;

	// Does not include padding bytes in comparison
	while (i < fs.data_length) {
		X = *(patch_data + sizeof(uint8_t) * i);
		Y = *(storage + sizeof(uint8_t) * i);

		if (X != Y) {		
			//printf("i: %u X: %1.2x Y: %1.2x\n\r ",i, X, Y);
			return false;
		}
		i++;
	}
	return true;
}
 