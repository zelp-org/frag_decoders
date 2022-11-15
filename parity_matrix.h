#pragma once

#include "bit_array.h"

void get_matrix_line(uint16_t n, uint16_t M, bit_array_t* C, uint8_t fraction);
uint32_t prbs23(uint32_t seed);