#include <libffi.h>

#include "c-call.h"


// external API
int do_ccall(Symbol* C_n, Type* r_t, List* a_t, size_t n, Value* a, Value* b);
