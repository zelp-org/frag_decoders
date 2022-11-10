#include "parity_matrix.h"
#include "bit_array.h"
#include <stdio.h>
#include <assert.h>




uint32_t prbs23(uint32_t x)
{
	uint32_t b0, b1;
	b0 = (x & 0x00000001);
	b1 = (x & 0x00000020) >> 5;
	return (x >> 1) + ((b0 ^ b1) << 22);
}


void get_matrix_line(uint16_t n, uint16_t M, bit_array_t* C) {
	assert(C != NULL);

	// clear all bits to start with
	clear_all_bits(C);
	//printf("n %u M %u\n\r", n, M);

	// uncoded messages for n <= M simply return identity diagonal
	if (n < M) {
		set_bit(C, n);
		return;
	}

	// coded mesagges need random parity matrix line containing 50% 1 and 50% 0
	uint32_t new_seed = 10011 * (n - M + 1) + 1;

	// Generate a line with approx M/2 bits sets to 1
	uint16_t numOnes = 0, rand_bit, max_loops=0;
	uint32_t next_seed;

	while (numOnes < (M / 2) && max_loops++ < M) {
		next_seed = prbs23(new_seed);
		rand_bit = prbs23(next_seed) % M;
		new_seed = next_seed;
		set_bit(C, rand_bit);
		numOnes = number_of_ones(C);
		//printf("rb: %u 1s: %u ", rand_bit, numOnes);
	}
	return;
}





