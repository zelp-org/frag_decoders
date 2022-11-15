#pragma warning(suppress : 6387)   // stupid 'memset "pointer could be 0' warning...

#include "storage.h"
#include "frag_sesh.h"
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include "constants.h"

/* **************************************************************************************
*										GLOBAL VARS
*  *************************************************************************************/
extern frag_sesh_t fs;
extern uint8_t* storage;


/* **************************************************************************************
*										PUBLIC FUNCTIONS
*  *************************************************************************************/
void create_storage(uint16_t num_bytes) {
	storage =  (uint8_t*)malloc( num_bytes * sizeof(uint8_t));
	memset(storage, 0x00, sizeof(uint8_t) * fs.M * fs.frag_size);
}

void store_fragment(uint16_t index, uint8_t* frag_data) {
	assert(frag_data != NULL);
	assert(storage != NULL);

	memcpy(storage + sizeof(uint8_t) * fs.frag_size * index, \
		frag_data, (size_t)fs.frag_size);
}

uint8_t* fetch_fragment(uint16_t index) {
	assert(storage != NULL);

	uint8_t* frag_start = (storage + sizeof(uint8_t) * ((uint16_t)fs.frag_size * index));
	return frag_start;
}

void free_storage() {
	assert(storage != NULL);
	free(storage);
}

bool check_for_null(void* ptr, int * return_code) {
	if (ptr == NULL) {
		*(return_code) = FAIL;
		return true;
	}
	*(return_code) = SUCCESS;
	return false;

}
