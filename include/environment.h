#ifndef rascal_environment_h
#define rascal_environment_h

#include "common.h"
#include "types.h"

// structure types ------------------------------------------------------------
struct module_t {
  value_t parent;
  value_t name;
  value_t doc;
  value_t namespace;
  value_t props;
  value_t values;
};

struct environment_t {
  value_t closure;

  uint_t length;
  uint_t cap;

  value_t *upvalues;
};

struct upvalue_t {
  value_t     closed;
  value_t     value;
  value_t     next;
};

// utilities ------------------------------------------------------------------
void trace_module(module_t *m);

// implementation -------------------------------------------------------------

// native functions -----------------------------------------------------------

// initialization -------------------------------------------------------------

#endif
