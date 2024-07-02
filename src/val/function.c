#include "val/function.h"
#include "val/environ.h"
#include "val/text.h"

#include "vm/state.h"

/* Globals */

/* External APIs */

/* General function APIs */
char* fn_name(void* fn) {
  Func* f = fn;

  return f->name->name->chars;
}

size_t fn_argc(void* fn) {
  Func* f = fn;

  return f->argc;
}

size_t fn_lvarc(void* fn) {
  Func* f = fn;

  return f->lvarc;
}

size_t fn_fcnt(void* fn) {
  Func* f = fn;
  size_t o;

  switch ( f->clt ) {
    case F_NONE:  o = 0; break;
    case F_CATCH: o = 5; break;
    case F_HNDL:  o = 5; break;
    default:      o = 3; break;
  }

  return o;
}

size_t fn_fsize(void* fn) {
  Func* f  = fn;
  size_t o = f->argc + f->lvarc;

  switch ( f->clt ) {
    case F_NONE:          break;
    case F_CATCH: o += 5; break;
    case F_HNDL:  o += 5; break;
    default:      o += 3; break;
  }

  return o;
}
