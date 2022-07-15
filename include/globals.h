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

/*
  stack layout:

  +-------+-------+-------+-------+-------+-------+-------+
  |  fun  | arg-n |  env  | savfp | argco | progc | loc-n |
  +-------+-------+-------+-------+-------+-------+-------+
                  ^                                       ^
                  |                                       |
	         Fp                                      Sp

  argco - offset to function object
  savfp - frame pointer to restore on exit
  progc - program counter

*/   

extern value_t Stack[N_STACK];
extern int Sp, Fp, Bp, Pc;
extern short *Instr;
extern value_t *Upval, *Value;
extern value_t UpvList;

#define Tos     (Stack[Sp-1])
#define Peek(n) (Stack[Sp-(n)])
#define Func    (Stack[Bp])
#define Env     (Stack[Fp])
#define SavFp   (Stack[Fp+1])
#define Argc    (Stack[Fp+2])
#define Progc   (Stack[Fp+3])

#define Unbound ((value_t)tag_symbol)

// heaps and heap state
extern uchar *Heap, *Reserve;

extern size_t  HSize, HUsed, RSize, RUsed;

extern bool  Collecting, Grow, Grew;

extern float Collectf, Resizef, Growf;

// error handling
extern jmp_buf Toplevel;
 
#endif
