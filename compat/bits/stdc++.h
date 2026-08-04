// Compatibility shim so the project builds with clang/libc++ (macOS),
// where GCC's <bits/stdc++.h> does not exist. Added to the include path
// for non-MSVC builds in CMakeLists.txt.
#pragma once

#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <numeric>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// x86/MSVC bit intrinsics used by the engine, mapped to compiler builtins
// on architectures (e.g. Apple Silicon / arm64) that don't provide them.
#if defined(__GNUC__) && !defined(__x86_64__)
inline unsigned long long _tzcnt_u64(unsigned long long x) {
    return x ? (unsigned long long)__builtin_ctzll(x) : 64;
}
inline long long __popcnt64(unsigned long long x) {
    return __builtin_popcountll(x);
}
#endif
