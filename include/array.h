#ifndef rascal_array_h
#define rascal_array_h

#include "common.h"
#include "types.h"

// structure types ------------------------------------------------------------
struct tuple_t {
  uint_t   length;
  ushort_t flags;
  ushort_t tag;

  value_t  data[0];
};

struct binary_t {
  uint_t   length;
  ushort_t Ctype;
  ushort_t tag;

  schar_t  data[0];
};

// utilities ------------------------------------------------------------------
value_t tuple_ref(value_t t, uint_t i);

#endif
