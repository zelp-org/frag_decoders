
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

extern bit_array_t A;
extern bit_array_t B;
extern bit_array_t C;
extern bit_array_t R;

/* **************************************************************************************
*										PRIVATE VARS & FUNCS
*  *************************************************************************************/
static void XOR_bytes_with_bit_array(bit_array_t* X, uint8_t* Y);


/* **************************************************************************************
*										PUBLIC FUNCTIONS
*  *************************************************************************************/
int init_decoder() {
	assert(storage != NULL);

	// fragments to be received up to packet_num=(M-1) are uncoded, so 
	// just note down if they were received in a bit array
	if (!get_bit_array(&R, fs.M)) {
		return FAIL ;
	}


	// get a bit array to hold indices of missing, but required uncoded fragments A
	if (!get_bit_array(&A, fs.M)) {
		return FAIL;
	}

	// get a bit array to hold indices of received and required uncoded fragments B
	if (!get_bit_array(&B, fs.M)) {
		return FAIL;
	}

	// get a bit array to hold a parity matrix line C
	if (!get_bit_array(&C, fs.M)) {
		return FAIL;
	}

	return SUCCESS;
}



int decode_fragment(uint8_t* frag, uint16_t packet_num) {
	assert(frag != NULL);
	uint16_t i, j, missing_index, num_frags_already_got;

	// not sure why I need to do this...
	packet_num = packet_num - 1;

	// process each received packet one by one, based on its number
	if (packet_num < fs.M) {
		// mark this uncoded packet as received
		set_bit(&R, packet_num);

		// store it to the right place in the store
		store_fragment(packet_num, frag);
		TRACE("stored uncoded fragment: %u\n\r", packet_num);
	}
	else {
		// for encoded fragments with packet number > M, get its corresponding parity matrix line
		get_matrix_line(packet_num, fs.M, &C, PARITY_LINE_FRACTION);
		print_bit_array(&C);

		// create B = C AND R, the indices of fragment in store , and that are required for this current frag
		copy_bit_array(&C, &B);
		bitwise_AND(&B, &R);
		print_bit_array(&B);

		// create A = A XOR C, the indices of fragment not in store, but that are required 
		copy_bit_array(&C, &A);
		bitwise_XOR(&A, &B);
		print_bit_array(&A);

		print_bit_array(&R);

		// if numbits A is one then reconstruct the one missing uncoded fragment with index in A
		if (number_of_ones(&A) == 1) {
			missing_index = index_of_first_one(&A);
			num_frags_already_got = number_of_ones(&B);
			TRACE("index in A: %u ", missing_index);
			
			for (i = 0; i < num_frags_already_got; i++) {
				// get index of next fragment to XOR in B
				j = index_of_first_one(&B);
				// if this is not  -1 we do have fragments to XOR 
				if (j != -1) {
					// mark this index as processed (0)
					clear_bit(&B, j);
					// XOR this uncoded fragment with the coded one
					bitwise_array_XOR(frag, fetch_fragment(j), fs.frag_size);
					TRACE("iB: %u ", j);
				}
				
			}
			TRACE("\n\r");
			// and store this reconstructed fragment 
			store_fragment(missing_index, frag);
			// and also mark in R as received and reconstructed
			set_bit(&R, missing_index);
			TRACE("reconstructed missing fragment: %u\n\r", missing_index);
		}

	}
	// if R is now all ones we are done, otherwise wait (with timeout) for more fragments 
	// that might never come!!!
	if (number_of_ones(&R) == fs.M) {
		TRACE("\n\r**** Decoding completed OK! ****\n\r");
		return DECODING_COMPLETE;
	}
	return DECODING_NOT_YET_COMPLETE;
}


bool is_decoded_same_as_original(uint8_t* patch_data) {
	uint16_t i = 0;
	uint8_t X, Y;

	// Does not include padding bytes in comparison
	while (i < fs.data_length) {
		X = *(patch_data + sizeof(uint8_t) * i);
		Y = *(storage + sizeof(uint8_t) * i);

		if (X != Y) {		
			TRACE("i: %u X: %1.2x Y: %1.2x\n\r ",i, X, Y);
			return false;
		}
		i++;
	}
	return true;
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
	free_bit_array(Yba);
	return;
}
 