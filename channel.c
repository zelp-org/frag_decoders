#include "channel.h"
#include "parity_matrix.h"
#include <time.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

uint16_t receive_next_fragment(uint8_t* frag_out, uint8_t* encoded_patch_data, uint16_t N, uint16_t frag_size, uint8_t packet_delivery_rate) {
	assert(encoded_patch_data != NULL);
	assert(frag_out != NULL);

	// counter for the number of packets received so far (max ~= N * packet_delivery_rate / 100)
	static packet_count = 0;
	static uint32_t rand_num;

	bool packet_lost;
	time_t seconds_since = time(NULL);
	int i;

	
	// initial seed on first entry
	if (packet_count == 0) {
		// just some made up combination based on seconds since 1st Jan 1970
		rand_num = prbs23((uint32_t)seconds_since / 137 + (uint32_t)seconds_since % 86400);
	}
	
	do {
		// with a chance of packet_delivery_rate %, output the next fragment 
		rand_num = prbs23(rand_num + 10111);
		packet_lost = (rand_num % 100) > packet_delivery_rate;
		
		if (!packet_lost) {
			// copy the mesage to the fragment output memory
			frag_out = memcpy(frag_out, encoded_patch_data + sizeof(uint8_t) * frag_size * packet_count, frag_size);

			printf("n=%1.3u data=", packet_count);
			for (i = 0; i < frag_size; i++) {
				printf("%1.2x ", *(frag_out + sizeof(uint8_t) * i));
			}
			printf("\n\r");

			return packet_count++;
		}
		else {
			// skip to the next packet
			printf("packet lost: %u\n\r", packet_count);
			packet_count++;		
		}
	} while (packet_count <= N);

	// may drop through
	return packet_count;
}