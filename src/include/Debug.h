#pragma once

#include "Log.h"

#ifndef NDEBUG
#define DEBUG(x) do { \
    x; \
} while (0)
#else
#define DEBUG(x)
#endif

#define DEBUG_LOG(...) DEBUG(LOG(__func__, __VA_ARGS__))