#ifndef rascal_globals_h
#define rascal_globals_h

#include <setjmp.h>

#include "common.h"
#include "types.h"

// globals --------------------------------------------------------------------
#define N_STACK 16384
#define N_HEAP  N_STACK

// global tables
extern ulong Symcnt;

extern symbols_t *Symbols;

// stacks and stack state
extern value_t Stack[N_STACK];
extern index_t Sp, Fp, Pc, Bp;

#define Tos     Stack[Sp-1]
#define Sref(n) Stack[Sp-(n)]

// heaps and heap state
extern uchar *Heap, *Reserve;

extern size_t  HSize, HUsed, RUsed;

extern bool  Collecting, Grow, Grew;

extern float Collectf, Resizef, Growf;

// error handling
extern jmp_buf Toplevel;
 
#endif
