#pragma once

#include <stdint.h>

uint16_t receive_next_fragment(uint8_t* frag_out, uint8_t* encoded_patch_data, uint16_t N, uint16_t frag_size, uint8_t packet_delivery_rate);