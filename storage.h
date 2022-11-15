#pragma once

#include <stdint.h>
#include <stdbool.h>


void create_storage(uint16_t num_bytes);
void store_fragment(uint16_t index, uint8_t* frag_data);
uint8_t* fetch_fragment(uint16_t index);
void free_storage(void);
bool check_for_null(void* ptr, int* return_code);
