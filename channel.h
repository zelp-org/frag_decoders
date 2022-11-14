#pragma once

#include <stdint.h>

uint16_t receive_next_fragment(uint8_t* frag_out, uint8_t* encoded_patch_data);