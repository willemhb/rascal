#ifndef rl_util_amt_h
#define rl_util_amt_h

#include "values.h"

/*
 * Common definitions and utilities for types that use an Array Mapped Trie storage model.
 *
 * Rascal AMT's have a branching factor of 64. This may change if it proves un-performant,
 * but it makes the math a little cleaner and makes hash collisions substantially less likely.
 * 
 */
/* C types */

/* Globals */
#define HT_MINC   8ul
#define HT_MAXC  64ul
#define HT_MAXS  (HT_MAXC*sizeof(void*))
#define HT_MASK  63ul
#define HT_SHFT   6ul
#define HT_MAXD   8ul

/* API */
size64 ht_index_for(size64 idx, size64 shift);
void   ht_copy(void* dst, void* src, size64 cnt);

/* Initialization */

#endif
