#ifndef rl_vm_obj_support_symbol_table_h
#define rl_vm_obj_support_symbol_table_h

#include "rascal.h"

#include "tpl/decl/hashmap.h"

/* commentary

   the Symbol table, duh. */

/* instantiations */
HASHMAP(symbol_table, char*, symbol_t*);

/* C types */

/* globals */
extern symbol_table_t SymbolTable;
extern ulong SymbolCounter;

/* API */
symbol_t *intern_string(char *string);
void      mark_symbol_table(symbol_table_t *symbol_table);

/* runtime */
void rl_vm_obj_support_symbol_table_init( void );
void rl_vm_obj_support_symbol_table_mark( void );

#endif
