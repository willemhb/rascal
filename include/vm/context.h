#ifndef rl_vm_context_h
#define rl_vm_context_h

#include "val/object.h"

/* Global environment state */

/* C types */

/* Specialized structure used to store interned strings. */
typedef struct StringTable StringTable;

struct StringTable {
  String** data;
  size_t   cnt;
  size_t   cap;
  size_t   nts;
};

/* Composite of all global environment state. */
struct RlContext {
  StringTable* string_cache;
  MutDict*     module_name_cache;
  MutDict*     module_path_cache;
  Module*      toplevel_module;
  idno_t       gensym_counter;
};

/* Globals */
extern RlContext Context;

/* External API */
String* intern_string(const char* chars, String* obj);
void    disintern_string(String* s);

/* Initialization */

#endif
