#ifndef rascal_globals_h
#define rascal_globals_h

#include <setjmp.h>

#include "common.h"
#include "types.h"

// globals --------------------------------------------------------------------
#define N_STACK 8192
#define N_HEAP  N_STACK

extern ulong   Symcnt;

// global tables
extern root_t Symbols, Syntax, Characters;
extern value_t Error, Ins, Outs, Errs;

// stacks and stack state
extern value_t Stack[N_STACK], Dump[N_STACK];
extern index_t Sp, Bp, Fp, Dp;

// heaps and heap state
extern uchar *Heap, *Reserve, *Free;

extern size_t  HeapSize, HeapUsed, RSize, RUsed;

extern bool  Collecting, Grow, Grew;

extern float Collectf, Resizef, Growf;

// error handling
extern jmp_buf Toplevel;

#endif
