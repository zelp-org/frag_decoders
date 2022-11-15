#pragma once

#include <stdint.h>

typedef struct frag_sesh_t {
	// data
	uint8_t frag_size;
	uint16_t M;
	uint16_t N;
	uint16_t data_length;
	uint8_t padding_bytes;
	float coding_rate;
	uint8_t pdr;
	uint8_t passes;
	uint8_t parity_fraction;
} frag_sesh_t;

void calc_padding(struct frag_sesh_t* fs);
void calc_N(struct frag_sesh_t* fs);
void print_session(void);