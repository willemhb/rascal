#ifndef rascal_symbol_h
#define rascal_symbol_h

#include "core.h"
#include "table.h"
#include "string.h"

struct symbol_t {
  object_t base;
  ulong_t hash, idno;
  value_t bind;

  char_t name[1];
};

#define assym(x)   ((symbol_t*)ptr(x))
#define symhash(x) (assym(x)->hash)
#define symidno(x) (assym(x)->idno)
#define symbind(x) (assym(x)->bind)
#define symloc(x)  (assym(x)->location)
#define symname(x) (assym(x)->name)

// api ------------------------------------------------------------------------
value_t symbol(char_t *name);
bool_t  symbolp(value_t xv);

#endif
