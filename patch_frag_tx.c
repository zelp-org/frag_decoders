/*
        PATCH FRAG TX  (O.Bailey Nov '22)
        Program to demonstrate the LoRaWAN file fragmentation session that Zelp AWS
        servers will need to implement.

        This C application runs on a Linux PC or in WSL on Windows, connected to a Zelp datalogger via a USB-UART

        The app creates a patch file consisting of a repeating counting sequence of bytes,
        along with a header that includes a hash check.

        It then applies the Forward Error Correction algorithm proposed in 
        https://lora-alliance.org/wp-content/uploads/2020/11/fragmented_data_block_transport_v1.0.0.pdf

        The app will then send the fragments over the UART to the device, but with a specified, random
        packet loss to simulate real world LoRa radio channel conditions.
*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <io.h>
#include "sha256.h"

typedef struct _patch_file_header_t
{
	uint32_t patch_len;
	uint8_t data_hash[32];
} patch_file_header_t;

int main(int argc, char* argv[])
{
	int ret = -1;

  // TBD: allow user to input a real patch file
	//FILE* hsrc = 0, 

  // output the generated patch file to compare to memeory contents of reconstructed patch in device memory
  FILE* pf_out = 0;  


	uint32_t len;
	uint8_t* p_buf;
	uint8_t fragment_size;
	uint8_t packet_delivery_rate;
	uint16_t pf_len;
	char portname[32];
	uint8_t port;
  HANDLE hport = 0;

  // check correct number of arguments
  if (argc != 6)
	{
		printf("usage:\n");
		printf("patch_frag_tx.exe [patch output file] [patch_file_length] \
					 [fragment size] [packet delivery rate %%] [COM port]\n");
		goto end;
	}

	// check that patch output file can be opened
  pf_out = fopen(argv[1], "wb");
	if (!pf_out)
	{
		printf("could not open file for patch output\n");
		goto end;
	}

	// check that patch file length format is OK
	if (1 != sscanf(argv[2], "%u", &pf_len))
	{
		printf("could not parse patch file size\n");
		goto end;
	}

	// check that fragment size format is OK
	if (1 != sscanf(argv[3], "%hhu", &fragment_size))
	{
		printf("could not parse fragment size\n");
		goto end;
	}

	// check that PDR format is OK
		if (1 != sscanf(argv[4], "%hhu", &packet_delivery_rate))
	{
		printf("could not parse packet delivery rate\n");
		goto end;
	}

	// check that COM port format is OK
		if (1 != sscanf(argv[5], "%hhu", &port))
	{
		printf("could not parse COM port\n");
		goto end;
	}

	// check limits on patch file size
	if (pf_len < 512 | pf_len > 24576) {
		printf("fragment size out of limits [512...24576 bytes]\n\r");
		goto end;
	}

	// check limits on fragment size
	if (fragment_size < 8 | fragment_size > 48) {
		printf("fragment size out of limits [8...48 bytes ]\n\r");
		goto end;
	}

	// check limits of PDR
	if (packet_delivery_rate < 33 | packet_delivery_rate > 100) {
		printf("packet delivery rate out of limits [33..100 %%]\n\r");
	}

  // create patch file with simple repeating sawtooth sequence {0..255,0..255...}
	uint32_t patch_file_size = ((len + 1023) / 1024) * 1024; 	 // align len to 1KB block
	p_buf = (uint8_t*)malloc(pf_len + sizeof(patch_file_header_t));
	memset(p_buf, 0, pf_len);
	if (!p_buf)
	{
		printf("could not alloc memory for patch file buffer\n");
		goto end;
	}

	// create sequence data
	// TBD
 
	// apply hash as header, along with length of patch file data
	patch_file_header_t* header = (patch_file_header_t*)p_buf;
	header->patch_len = pf_len;

	SHA256_CTX hasher;
	sha256_init(&hasher);
	sha256_update(&hasher, buf + sizeof(patch_file_header_t), pf_len);
	sha256_final(&hasher, header->data_hash);

	if (pf_len + sizeof(patch_file_header_t) != fwrite(p_buf, 1, pf_len + sizeof(patch_file_header_t), pf_out))
	{
		printf("could not write to patch output file\n");
		goto end;
	}

  // save the patch file
	 fwrite (p_buf , sizeof(uint8_t), sizeof(p_buf), pf_out);

	// fragment and apply FEC whilst outputting each fragment on the UART


	printf("ok\n");
	ret = 0;

end:
	/*if (hsrc)
		fclose(hsrc);*/
    
	if (pf_out)
		fclose(pf_out);
	return ret;
}

