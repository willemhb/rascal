#ifndef rascal_binary_h
#define rascal_binary_h

#include "ctypes.h"
#include "rascal.h"
#include "describe/array.h"

struct binary_t ArrayType(uchar_t, flags, 0);

// api ------------------------------------------------------------------------
value_t mk_binary(void_t *data, size_t n);

#endif
