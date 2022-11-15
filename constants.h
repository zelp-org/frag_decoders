#pragma once
#include <stdbool.h>

#define TRACE( message, ... )   if (trace_on) printf(message, __VA_ARGS__ )

extern const int FAIL;
extern const int SUCCESS;
extern const int DECODING_COMPLETE;
extern const int DECODING_NOT_YET_COMPLETE;

extern const int PARITY_LINE_FRACTION;

extern bool trace_on;