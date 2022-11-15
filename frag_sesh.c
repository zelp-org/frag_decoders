#include "frag_sesh.h"
#include "constants.h"
#include <stdio.h>

extern frag_sesh_t fs;

void calc_padding(struct frag_sesh_t* fs) {
	fs->padding_bytes = fs->M * fs->frag_size - fs->data_length;
	if (fs->padding_bytes == fs->frag_size) {
		fs->padding_bytes = 0;
	}
}


void calc_N(struct frag_sesh_t* fs) {
	fs->N = (uint16_t)((float)fs->M * fs->coding_rate);
}

void print_session(void) {
	TRACE("\n\rM: %u N: %u Padding: %u Data length: %u Fragment Size: %u PDR: %u Passes: %u \
Parity Fraction: %u\n\r",\
		fs.M, fs.N, fs.padding_bytes, fs.data_length, fs.frag_size, fs.pdr, fs.passes, fs.parity_fraction);
}