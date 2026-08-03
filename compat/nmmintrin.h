// Compatibility shim for <nmmintrin.h> (SSE4.2) on non-x86 targets.
#pragma once

#if defined(__x86_64__) || defined(_M_X64)
#include_next <nmmintrin.h>
#else
inline long long _mm_popcnt_u64(unsigned long long x) {
    return __builtin_popcountll(x);
}
#endif
