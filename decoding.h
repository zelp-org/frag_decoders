#pragma once

#include <stdint.h>
#include <stdbool.h>

int init_decoder(void);
int decode_fragment(uint8_t* frag, uint16_t packet_num);
bool is_decoded_same_as_original(uint8_t* patch_data);
void final_step(void);