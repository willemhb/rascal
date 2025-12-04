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


// initialization -------------------------------------------------------------

#endif
