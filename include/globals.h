#ifndef rascal_globals_h
#define rascal_globals_h

#include <setjmp.h>

#include "common.h"
#include "rtypes.h"

// globals --------------------------------------------------------------------
#define N_STACK 8192
#define N_HEAP  N_STACK

extern ulong_t   Symcnt;

// global tables
extern object_t *Symbols, *Syntax, *Characters;

// other object types
extern object_t *Ins, *Outs, *Errs;

// stacks and stack state
extern value_t Stack[N_STACK], Dump[N_STACK];
extern index_t Sp, Bp, Fp, Dp;

// heaps and heap state
extern uchar_t *Heap, *Reserve, *Free;

extern size_t  NHeap, HeapUsed, HeapSize, NReserve, ReserveUsed, ReserveSize;

extern bool_t  Collecting, Grow, Grew;

extern float_t Collectf, Resizef, Growf;

// error handling
extern jmp_buf Toplevel;

#endif
