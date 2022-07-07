#ifndef rascal_globals_h
#define rascal_globals_h

#include "common.h"
#include "rtypes.h"

// globals --------------------------------------------------------------------
#define N_STACK 8192
#define N_HEAP  N_STACK

extern ulong_t   Symcnt;
extern object_t *Symbols, *Globals, *Error, *Ins, *Outs, *Errs;

extern value_t Stack[N_STACK], Dump[N_STACK], Function;

extern index_t Sp, Bp, Fp, Dp;

extern uchar_t *Heap, *Reserve, *Free, *MapFree, *HeapMap, *ReserveMap;

extern size_t  NHeap, HeapUsed, HeapSize, NReserve, ReserveUsed, ReserveSize;

extern bool_t  Collecting, Grow, Grew;

extern float_t Collectf, Resizef, Growf;

// utilities ------------------------------------------------------------------

#endif
