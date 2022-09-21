#include <string.h>

#include "exec.h"
#include "mem.h"
#include "obj/num.h"
#include "obj/obj.h"
#include "template/array.h"

#define min_stack_arity 128ul

// stack implementation
typedef ARRAYLIST(stack, val_t) stack_t;

INIT_ALIST(stack, val_t)
