/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2009 Marco Costalba

  Stockfish is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.


  Stockfish is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#if !defined(BITCOUNT_H_INCLUDED)
#define BITCOUNT_H_INCLUDED

// To disable POPCNT support uncomment following line. You should do it only
// in PGO compiling to exercise the default fallback path. Don't forget to
// re-comment the line for the final optimized compile though ;-)
//#define DISABLE_POPCNT_SUPPORT


#include "types.h"

// Select type of intrinsic bit count instruction to use

#if defined(_MSC_VER) && defined(_WIN64) // Microsoft compiler

#include <intrin.h>

inline bool cpu_has_popcnt() {

  int CPUInfo[4] = {-1};
  __cpuid(CPUInfo, 0x00000001);
  return (CPUInfo[2] >> 23) & 1;
}

#define POPCNT_INTRINSIC(x) __popcnt64(x)

#elif defined(__INTEL_COMPILER) && (defined(__x86_64) || defined(_M_X64)) // Intel compiler

#include <nmmintrin.h>

inline bool cpu_has_popcnt() {

  int CPUInfo[4] = {-1};
  __cpuid(CPUInfo, 0x00000001);
  return (CPUInfo[2] >> 23) & 1;
}

#define POPCNT_INTRINSIC(x) _mm_popcnt_u64(x)

#else // Safe fallback for unsupported compilers

inline bool cpu_has_popcnt() { return false; }

#define POPCNT_INTRINSIC(x) count_1s(x)

#endif // cpu_has_popcnt() selection


/// Software implementation of bit count functions

#if defined(IS_64BIT)

inline int count_1s(Bitboard b) {
  b -= ((b>>1) & 0x5555555555555555ULL);
  b = ((b>>2) & 0x3333333333333333ULL) + (b & 0x3333333333333333ULL);
  b = ((b>>4) + b) & 0x0F0F0F0F0F0F0F0FULL;
  b *= 0x0101010101010101ULL;
  return int(b >> 56);
}

inline int count_1s_max_15(Bitboard b) {
  b -= (b>>1) & 0x5555555555555555ULL;
  b = ((b>>2) & 0x3333333333333333ULL) + (b & 0x3333333333333333ULL);
  b *= 0x1111111111111111ULL;
  return int(b >> 60);
}

#else // if !defined(IS_64BIT)

inline int count_1s(Bitboard b) {
  unsigned w = unsigned(b >> 32), v = unsigned(b);
  v -= (v >> 1) & 0x55555555; // 0-2 in 2 bits
  w -= (w >> 1) & 0x55555555;
  v = ((v >> 2) & 0x33333333) + (v & 0x33333333); // 0-4 in 4 bits
  w = ((w >> 2) & 0x33333333) + (w & 0x33333333);
  v = ((v >> 4) + v) & 0x0F0F0F0F; // 0-8 in 8 bits
  v += (((w >> 4) + w) & 0x0F0F0F0F);  // 0-16 in 8 bits
  v *= 0x01010101; // mul is fast on amd procs
  return int(v >> 24);
}

inline int count_1s_max_15(Bitboard b) {
  unsigned w = unsigned(b >> 32), v = unsigned(b);
  v -= (v >> 1) & 0x55555555; // 0-2 in 2 bits
  w -= (w >> 1) & 0x55555555;
  v = ((v >> 2) & 0x33333333) + (v & 0x33333333); // 0-4 in 4 bits
  w = ((w >> 2) & 0x33333333) + (w & 0x33333333);
  v += w; // 0-8 in 4 bits
  v *= 0x11111111;
  return int(v >> 28);
}

#endif // BITCOUNT


/// count_1s() counts the number of nonzero bits in a bitboard.
/// If template parameter is true an intrinsic is called, otherwise
/// we fallback on a software implementation.

template<bool UseIntrinsic>
inline int count_1s(Bitboard b) {

  return UseIntrinsic ? POPCNT_INTRINSIC(b) : count_1s(b);
}

template<bool UseIntrinsic>
inline int count_1s_max_15(Bitboard b) {

  return UseIntrinsic ? POPCNT_INTRINSIC(b) : count_1s_max_15(b);
}


// Global variable initialized at startup that is set to true if
// CPU on which application runs supports POPCNT intrinsic. Unless
// DISABLE_POPCNT_SUPPORT is defined.
#if defined(DISABLE_POPCNT_SUPPORT)
const bool CpuHasPOPCNT = false;
#else
const bool CpuHasPOPCNT = cpu_has_popcnt();
#endif


// Global variable used to print info about the use of 64 optimized
// functions to verify that a 64bit compile has been correctly built.
#if defined(IS_64BIT)
const bool CpuHas64BitPath = true;
#else
const bool CpuHas64BitPath = false;
#endif

#endif // !defined(BITCOUNT_H_INCLUDED)
