#pragma warning(suppress : 6387)   // stupid 'memset "pointer could be 0' warning...

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>	
#include <stdlib.h>
#include "bit_array.h"
#include "parity_matrix.h"
#include "storage.h"
#include "channel.h"
#include "encoding.h"
#include "decoding.h"
#include "constants.h"
#include "frag_sesh.h"
#include "check_arguments.h"

/*
* LoRaWAN Fragmentation Encodder/Decoder Demonstration for Embedded with low memory usage
*
* O Bailey Nov 2022
*
* This demonstrates  the fragmentation encoding (server side) and decoding (device side)
* for a LoRaWAN file transfer (typically a new firmware  or firmware patch)
*
* The original data is a sawtooth pattern counting upwards between 0 to 255.
* 
* COMMAND LINE ARGUMENTS:
* ./patch_frag_tx.exe  [patch_file_length]  [fragment_size] [coding_rate] [packet_delivery_rate]
* 
* where:
* patch_file_length is the length in bytes of the raw patch file to be fragmented &  forward error corrected
* fragment_size is the size in bytes for each downlink fragment. Typically limited to 48 bytes
* coding_rate is the ratio of extra redundant information
* packet_delivery_rate is  the percentage of fragments successfully received on the downlink (max. 100%)
*
* 
* OPERATION OF THE FRAMENTATION PROCESS:
* 
* 'SERVER SIDE'
* 1. The patch file is split in M equal size fragments, each of length frag_size
* 2. (the final fragment is padded with zeros if necessary to keep length = frag_size)
* 3. In total, N fragments will be created, where N = M * coding_rate
* 4. The extra (N-M) fragments will be parity encoded by XORing a random combination
*    of about 50% of the M fragments.
* 5. The combination of M fragments to make each of the (N-M) coded fragments is known
*    to both the server and device, as they will use the same pseudo-randon sequence generator
*    and seed numbers to encoded and decode the coded fragments
* 
*  'RADIO CHANNEL'
* 6. All N fragments are transmitted in numerical order. The first M fragments are the raw, uncoded
*    fragments, followed by (N-M) coded fragments.
* 7. Some fragments are randomly lost due to noise, interference, collisions etc. 
* 
*  'DEVICE'
* 8. The device prepares storage memory (S) large enough to hold the patch file and sets all memory to zero.
* 9. The device receives (packet_delivery_rate/100)*N fragments. If the number of lost fragments
*    is a bit less than the extra redundancy added, we are likely to be able to recover the original
*    patch file.
* 10. The device receives all or some (uncoded) fragments 1 thru M, making a note of which fragment numbers 
*    were received in R, and storing the fragments in order in storage memory S
* 11. If all M fragments are received, the process can stop, since the original data is now fully known.
* 12. If some of the M fragments are not received, the device must then receive the (N-M) coded fragments
* 13. For each of the (n)th coded fragments that arrives, the device will:
*			a) Retrieve the combination (C) of the indexs of the uncoded messages used to build this (n)th 
*			b) XOR all indexes in storage S with the fragment n 
*			c) Store this XORd value to the first index not yet marked as received
*			d) If only one index was not received, we now have recreated it, so mark it as received in R
*			e) But if more than one index was not received, create a parity line A (???)
*/


const int FAIL = -1;
const int SUCCESS = 0;

/* **************************************************************************************
*										GLOBAL VARS
*  *************************************************************************************/
frag_sesh_t fs = { 0 };
uint8_t	  packet_delivery_rate = 0;
uint8_t* storage;

bit_array_t* tally_of_rxd_frags;
bit_array_t C = { 0 };
uint8_t* A;


/* **************************************************************************************
*											MAIN 
*  *************************************************************************************/ 
int main(int argc, char* argv[]) {
	int		  i;
	int		  ret = FAIL;
	
	uint8_t*  patch_data = NULL;
	uint8_t*  frag = NULL;
	uint8_t*  FEC_data = NULL;
	uint16_t  packet_num = 0;

	
	// check arguments and populate fragmentation session structure 'fs'
	check_arguments(argc, argv);
	
	// create patch data (with padding)
	patch_data = (uint8_t*)malloc(sizeof(uint8_t) * fs.M * fs.frag_size);
	if (check_for_null((void*)patch_data, &ret)) goto end;
	
	// initially set the patch data to all zeros
	memset(patch_data, 0x00, (size_t)(fs.M * fs.frag_size));

	// create the sequence data for the raw "patch file"
	for (i = 0; i < fs.data_length; i++) {
		*(patch_data + i) = i & 0xFF;
	}

	// allocate memory for encoded output data
	FEC_data = (uint8_t*)malloc((size_t)(sizeof(uint8_t) * fs.N * fs.frag_size));
	if (check_for_null((void*)FEC_data, &ret)) goto end;

	memset(FEC_data, 0x00, (size_t)(fs.N * fs.frag_size));

	// encode the patch data, expanding it with redundancy in (N-M) extra fragments
	encode(patch_data, FEC_data, fs.N, fs.M, fs.frag_size);

	// memory to store a received fragment for processing
	frag = (uint8_t*) malloc(sizeof(uint8_t) * fs.frag_size);
	if (check_for_null((void*)frag, &ret)) goto end;

	// create the storage space for the received  and decoded fragments
	create_storage();


	// prepare local vars in the decoder
	if (init_decoder() == FAIL) goto end;
	
	// receive the messages (with some getting lost on the way) and decode one-by-one
	while (packet_num < (fs.N)) {
		// simulate a lossy LoRa radio link
		packet_num = receive_next_fragment(frag, FEC_data);
			
		// and pass each received message to the decoder as they arrive
		decode_fragment(frag, packet_num);
	} 
	final_step();


	// compare the received fragments (1..M) with the original data!
	if (is_decoded_same_as_original(patch_data) == true) {
		printf("\n\r *** Patch file received OK :-) ***\n\r");
	}
	else {
		printf("\n\r *** Patch file NOT received OK :-( ***\n\r");
	}
	
	ret = SUCCESS;

end:
	// free up allocated heap memory (it's OK to free a NULL pointer too if it wasn't used...)
	free(patch_data);
	free(frag);
	free(FEC_data);
	delete_bit_array(&C);
	delete_bit_array(tally_of_rxd_frags);
	delete_storage();


	// say goodbye
	printf("\n\rexiting: %i\n\r", ret);
	
	return ret;
}



