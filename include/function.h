#ifndef rascal_function_h
#define rascal_function_h

#include "common.h"
#include "types.h"

// structure types ------------------------------------------------------------

#define function_head					\
  uint_t   arity;					\
  uchar_t  vargs;					\
  uchar_t  macro;					\
  ushort_t tag;						\
  value_t   name;					\
  value_t   head

struct compiled_t {
  function_head;

  value_t envt;
  value_t vals;
  value_t code;
};

struct lambda_t {
  function_head;

  value_t envt;
  value_t args;
  value_t body;
};

struct generic_t {
  function_head;

  value_t zarg;
  value_t farg;
  value_t varg;
};

struct native_t {
  function_head;

  value_t (*handler)(value_t *args, size_t n_args);
};

struct control_t {
  uint_t    length;
  ushort_t  captured;
  ushort_t  tag;

  slong_t   offset; // offset (from end) to frame pointer
  value_t   *saved;  // saved stack frame slice
};

#endif
