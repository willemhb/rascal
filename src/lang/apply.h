#ifndef lang_apply_h
#define lang_apply_h

#include "types/value.h"

/* API */
Val apply(Val head, Val args);
Val invoke(Func *func, uint n, Val *args, List* opt, Table* kw, List* va);
Val call(Func *func, int n_args, Val *args, List* opt, Table* kw, List* va);

// convenience macros ---------------------------------------------------------

#endif
