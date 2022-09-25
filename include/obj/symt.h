#ifndef rascal_symt_h
#define rascal_symt_h

#include "obj.h"
#include "template/table.h"

typedef struct str_t  str_t;
typedef struct atom_t atom_t;

DECL_OBJ(symt);
DECL_OBJ_API(symt);
DECL_OBJ(symt_kv);
DECL_OBJ_API(symt_kv);

DECL_TABLE(symt, str_t*, atom_t*);
DECL_TABLE_API(symt, str_t*, atom_t*);

// globals --------------------------------------------------------------------
extern symt_t *SymbolTable;

#endif
