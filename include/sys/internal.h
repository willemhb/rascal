#ifndef rl_sys_internal_h
#define rl_sys_internal_h

/* Implementations for internal collection types. */

// headers --------------------------------------------------------------------
#include "common.h"
#include "sys/base.h"

// macros ---------------------------------------------------------------------
// C types --------------------------------------------------------------------
// globals --------------------------------------------------------------------
// function prototypes --------------------------------------------------------
/* symbol table */
void init_symbol_table(SymbolTable* st);
void reset_symbol_table(SymbolTable* st);
void free_symbol_table(SymbolTable* st);
Sym* intern_symbol(SymbolTable* st, char* name);

/* itable */
void init_itable(ITable* it, bool environ);
void reset_itable(ITable* it);
void free_itable(ITable* it);
bool itable_get(ITable* it, Expr k, Expr* b);
bool itable_set(ITable* it, Expr k, Expr v);
bool itable_del(ITable* it, Expr k, Expr* b);

/* ibuffer */
void init_ibuffer(IBuffer* ib, int elsize, bool encoded);
void reset_ibuffer(IBuffer* ib);
void free_ibuffer(IBuffer* ib);
void ibuffer_add8(IBuffer* ib, byte_t d);
void ibuffer_add16(IBuffer* ib, ushort_t d);
void ibuffer_write8(IBuffer* ib, lcount_t n, byte_t* d);
void ibuffer_write16(IBuffer* ib, lcount_t n, ushort_t* d);

#define ibuffer_add(ib, d)                                              \
  generic((d), byte_t: ibuffer_add8, ushort_t: ibuffer_add16)(ib, d)

#define ibuffer_write(ib, n, d)                                         \
  generic((d), byte_t*:ibuffer_write8, ushort_t*: ibuffer_write16)(ib, d)

/* istack */

// initialization -------------------------------------------------------------

#endif
