#pragma once

#include <stdint.h>
#include <stdbool.h>


uint8_t* create_storage(uint16_t M, uint16_t frag_size);
void store_fragment(uint16_t index, uint16_t frag_size, uint8_t* frag_data, uint8_t* storage);
uint8_t* fetch_fragment(uint16_t index, uint16_t frag_size, uint8_t* storage);
void delete_storage(uint8_t* storage);
bool check_for_null(void* ptr, int* return_code);
