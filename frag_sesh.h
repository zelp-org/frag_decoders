#pragma once

#include <stdint.h>

typedef struct frag_sesh_t {
	// data
	uint16_t frag_size;
	uint16_t M;
	uint16_t N;
	uint16_t data_length;
	uint8_t padding_bytes;
	float coding_rate;
} frag_sesh_t;

void calc_padding(struct frag_sesh_t* fs);
void calc_N(struct frag_sesh_t* fs);