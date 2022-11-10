#pragma once

#include <stdint.h>



typedef struct frag_sesh_t {
	// data
	uint16_t frag_size;
	uint8_t whole_bytes;
	uint8_t* array;
	// functions
	void (*clear_all_bits_func)(struct bit_array_t* bitarray);
	void (*set_bit_func)(struct bit_array_t* bitarray, uint16_t bitNum);
	void (*get_bit_func)(struct bit_array_t* bitarray, uint16_t bitNum);
	int16_t(*index_of_first_one_func)(struct bit_array_t* bitarray);
	void (*bitwise_AND_func)(struct bit_array_t* bitarray, struct bit_array_t* other_array);
	void (*bitwise_XOR_func)(struct bit_array_t* bitarray, struct bit_array_t* other_array);
} frag_sesh_t;

int init_decoder(uint8_t* storage, uint16_t frag_size, uint16_t M, uint16_t N);
void decode_fragment(uint8_t* frag, uint16_t packet_num);
void free_decoder_memory(void);