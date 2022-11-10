#pragma once

#include <stdint.h>

void encode(uint8_t* data_in, uint8_t* fec_data_out,uint16_t N, uint16_t M, uint8_t fragsize);
void bitwise_array_XOR(uint8_t* A, uint8_t* B, uint16_t num_bytes);