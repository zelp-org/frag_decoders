#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct bit_array_t {
	// data
	uint16_t numBits;
	uint8_t whole_bytes;
	uint8_t* array;
} bit_array_t;

bool get_bit_array(bit_array_t* bitarray, uint16_t numbits);
void copy_bit_array(bit_array_t* src, bit_array_t* dest);
void free_bit_array(bit_array_t* bitarray);
void print_bit_array(bit_array_t* b);

int16_t index_of_first_one(bit_array_t* bitarray);
void bitwise_AND(bit_array_t* bitarray, bit_array_t* other_array);
void bitwise_XOR(bit_array_t* bitarray, bit_array_t* other_array);
void set_bit(bit_array_t* bitarray, uint16_t bitNum);
bool get_bit(bit_array_t* bitarray, uint16_t bitNum);
void clear_bit(bit_array_t* bitarray, uint16_t bitNum);
void clear_all_bits(bit_array_t* bitarray);
uint16_t number_of_ones(bit_array_t* bitarray);


