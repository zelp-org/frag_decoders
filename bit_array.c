#include "bit_array.h"
#include "constants.h"
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

void clear_bit(bit_array_t* bitarray, uint16_t bitNum) {
    assert(bitarray != NULL);

    // which byte and bit position in that byte is this bit to set?
    if (bitNum >= bitarray->numBits) return;
    uint8_t byte_num = bitNum / 8;
    uint8_t bit_position = bitNum % 8;
    uint8_t bit_mask = 0x80 >> bit_position;
    // clear bit number 'bitNum' in the bit array
    *(bitarray->array + byte_num) = *(bitarray->array + byte_num) & ~((uint8_t)(0xFF) & bit_mask);
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
    b->array = malloc(sizeof(uint8_t) * b->whole_bytes);
    if (b->array != NULL) {
        b->array = memset(b->array, 0x00, sizeof(uint8_t) * b->whole_bytes);
        return true;
    }
    return false;  
}

// make a deep copy of a bit array
void copy_bit_array(bit_array_t * src, bit_array_t* dest) {
    dest->numBits = src->numBits;
    dest->whole_bytes = src->whole_bytes;
    memcpy(dest->array, src->array, dest->whole_bytes);

    return;
}

void free_bit_array(bit_array_t* bitarray) {
    assert(bitarray != NULL);

    // free up the memory used by this bit array
    free(bitarray->array);
    free(bitarray);

    return;
}

 // print array
 void print_bit_array(bit_array_t* b) {
     assert(b != NULL);

     int i;

     for (i = 0; i < b->whole_bytes; i++) {
         TRACE("0x%02x ", *(b->array + sizeof(uint8_t) * i));
     }
     TRACE("\n\r");

     return;
 }