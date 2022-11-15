#pragma once

#include <stdbool.h>

#define TRACE( message, ... )   if (trace_on) printf(message, __VA_ARGS__ )

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

extern const int FAIL;
extern const int SUCCESS;
extern const int DECODING_COMPLETE;
extern const int DECODING_NOT_YET_COMPLETE;


extern bool trace_on;

