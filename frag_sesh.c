#include "frag_sesh.h"

void calc_padding(struct frag_sesh_t* fs) {
	fs->padding_bytes = fs->M * fs->frag_size - fs->data_length;
	if (fs->padding_bytes == fs->frag_size) {
		fs->padding_bytes = 0;
	}
}


void calc_N(struct frag_sesh_t* fs) {
	fs->N = (uint16_t)((float)fs->M * fs->coding_rate);
}