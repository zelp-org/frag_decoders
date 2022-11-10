#include "storage.h"
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "constants.h"


uint8_t* create_storage(uint16_t M, uint16_t frag_size) {
	uint8_t* storage_start =  (uint8_t*)malloc(((uint16_t)(M * frag_size)) * sizeof(uint8_t));
	return storage_start;
}

void store_fragment(uint16_t index, uint16_t frag_size, uint8_t* frag_data, uint8_t* storage) {
	assert(frag_data != NULL);
	assert(storage != NULL);

	uint16_t i;

	for (i = 0; i < frag_size; i++) {
		*(storage + sizeof(uint8_t) * (frag_size * index + i)) = *(frag_data + sizeof(uint8_t) * i);
	}
}

uint8_t* fetch_fragment(uint16_t index, uint16_t frag_size, uint8_t* storage) {
	assert(storage != NULL);

	uint8_t* frag_start = (storage + sizeof(uint8_t) * (frag_size * index));
	return frag_start;
}

void delete_storage(uint8_t* storage) {
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
