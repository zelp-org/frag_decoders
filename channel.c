#include "channel.h"
#include "parity_matrix.h"
#include "constants.h"
#include "frag_sesh.h"
#include <time.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

/* **************************************************************************************
*										GLOBAL VARS
*  *************************************************************************************/
extern uint8_t packet_delivery_rate;
extern frag_sesh_t fs;


/* **************************************************************************************
*										PUBLIC FUNCTIONS
*  *************************************************************************************/
uint16_t receive_next_fragment(uint8_t* frag_out, uint8_t* encoded_patch_data) {
	assert(encoded_patch_data != NULL);
	assert(frag_out != NULL);

	// counter for the number of packets received so far (max ~= N * packet_delivery_rate / 100)
	static uint16_t packet_count = 0;
	static uint32_t rand_num;

	bool packet_lost;
	time_t seconds_since = time(NULL);
	int i;

	
	// initial seed on first entry
	if (packet_count == 0) {
		// just some made up combination based on seconds since 1st Jan 1970
		rand_num = prbs23((uint32_t)seconds_since / 137 + (uint32_t)seconds_since % 86400);
	}

next_packet:
	// with a chance of packet_delivery_rate %, output the next fragment 
	rand_num = prbs23(rand_num + 10111);
	packet_lost = (rand_num % 100) > packet_delivery_rate;

	// if the packet is not lost, then copy it out
	if (!packet_lost && (packet_count < fs.N)) {
		// copy the mesage to the fragment output memory
		frag_out = memcpy(frag_out, \
			encoded_patch_data + sizeof(uint8_t) * fs.frag_size * packet_count, fs.frag_size);

		TRACE("rx: %1.3u ", packet_count);
		for (i = 0; i < fs.frag_size; i++) {
			TRACE("%1.2x ", *(frag_out + sizeof(uint8_t) * i));
		}
		TRACE("\n\r");
		// packet wasn't lost, go see if we receive the next one
		packet_count += 1;
		return packet_count;
	}
	else {
		// packet was lost, so skip to the next packet, if there are still more
		TRACE(">>>>>\tpacket lost: %u\n\r", packet_count);
		packet_count += 1;
		if (packet_count < fs.N)
			goto next_packet;
		else
			return (fs.N + 1); // break loop by sending packet_num one more than we actually have!
	}
}