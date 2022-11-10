#include "bit_array.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>


// 'private' functions
 void clear_all_bits(bit_array_t* bitarray) {
    assert(bitarray != NULL);

    bitarray->array = memset(bitarray->array, 0, bitarray->numBits);
}

 void set_bit(bit_array_t* bitarray, uint16_t bitNum) {
    assert(bitarray != NULL);

    // which byte and bit position in that byte is this bit to set?
    if (bitNum >= bitarray->numBits) return;
    uint8_t byte_num = bitNum / 8;
    uint8_t bit_position = bitNum % 8;
    // set bit number 'bitNum' in the bit array
    *(bitarray->array + byte_num) = *(bitarray->array + byte_num) | (0x80 >> bit_position);
}

bool get_bit(bit_array_t* bitarray, uint16_t bitNum) {
    assert(bitarray != NULL);

    // get the bit value in position n
    if (bitNum  >= bitarray->numBits) return false;
    uint8_t byte_num = bitNum / 8;
    uint8_t bit_position = bitNum % 8;
    // get bit number 'bitNum' in the bit array
    uint8_t masked_byte =  (* (bitarray->array + byte_num) & (0x80 >> bit_position));
    if (masked_byte > 0) 
        return true;
    else 
        return false;
}

void bitwise_XOR(bit_array_t* this_array, bit_array_t* other_array) {
    assert(this_array != NULL);
    assert(other_array != NULL);

    // XOR the two arrays bit by bit, and store back in this array.
    uint16_t i;
    for (i = 0; i < this_array->whole_bytes; i++) {
        *(this_array->array + i) = *(this_array->array + i) ^ *(other_array->array + i);
    }
}

void bitwise_AND(bit_array_t* this_array, bit_array_t* other_array) {
    assert(this_array != NULL);
    assert(other_array != NULL);

    // AND the two arrays bit by bit, and store back in this array.
    uint16_t i;
    for (i = 0; i < this_array->whole_bytes; i++) {
        *(this_array->array + i) = *(this_array->array + i) & *(other_array->array + i);
    }
}

int16_t index_of_first_one(struct bit_array_t* bitarray) {
    assert(bitarray != NULL);

    // retrieve the index of the first bit in the bit array set to 1
    // returns -1 if all zeros 
    int16_t i = 0;
    uint8_t  j = 0, k = 0x80;

    // loop through the bytes of the bit array
    while (i < bitarray->whole_bytes) {
        // loop through the bits of each byte, MSB to LSB (left to right)
        while (j < 8) {
            if (k & *(bitarray->array + i)) {
                // return the postion of the bit in the array
                return ((8 * i) + j);
            }
            j++;
            k = k >> 1;
        }
        j = 0;
        k = 0x80;
        i++;
    }
    // must be all zeros
    return -1;
}


uint16_t number_of_ones(bit_array_t* b) {
    assert(b != NULL);

    uint16_t bits =  b->numBits % 8;
    if (bits == 0) bits = 8;
    uint8_t  bytes = b->whole_bytes;
    uint16_t ones = 0;
    uint8_t mask = 0x80;

    // count backwards from the last byte
    while (bytes > 0) {
        for (; bits > 0; bits--) {
            if (mask & *(b->array + sizeof(uint8_t) * (bytes - 1))) {
                ones++;
            }
            mask >>= 1;
        }
        bytes--;
        bits = 8;
        mask = 0x80;
    }  
    return ones;
}


// bitarray getter function
bool get_bit_array(bit_array_t* b, uint16_t numbits) {
    assert(b != NULL);

    if (numbits == 0) return false;
    
    b->numBits = numbits;
    b->whole_bytes = ((numbits-1) / 8) + 1;
    b->bitwise_AND_func = bitwise_AND;
    b->bitwise_XOR_func = bitwise_XOR;
    b->clear_all_bits_func = clear_all_bits;
    b->set_bit_func = set_bit;
    b->set_bit_func = get_bit;
    b->index_of_first_one_func = index_of_first_one;
    b->array = malloc(sizeof(uint8_t) * b->whole_bytes);
    if (b->array != NULL) {
        b->array = memset(b->array, 0x00, sizeof(uint8_t) * b->whole_bytes);
        return true;
    }
    return false;  
}

void delete_bit_array(bit_array_t* bitarray) {
    assert(bitarray != NULL);

    // free up the memory used by this bit array
    free(bitarray->array);
    free(bitarray);
}

 // print array
 void print_bit_array(bit_array_t* b) {
     assert(b != NULL);

     int i;

     for (i = 0; i < b->whole_bytes; i++) {
         printf("0x%02x ", *(b->array + sizeof(uint8_t) * i));
     }
     printf("\n\r");
 }