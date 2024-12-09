#include <math.h>

#include "vm/error.h"

#include "util/collection.h"
#include "util/number.h"
#include "util/memory.h"


/* C types */
/* Forward declarations */
/* Globals */
#define MIN_CAP 8ul
#define LF      0.625
#define LF_I    1.6

/* API */
bool check_alist_grow(size64 m, size64 n) {
  return n > m;
}

bool check_alist_shrink(size64 m, size64 n) {
  return m > MIN_CAP && n < (m >> 1);
}

// same as check_alist_grow/shrink but pads n for encoded types
bool check_buffer_grow(size64 m, size64 n) {
  return n + 1 > m;
}

bool check_buffer_shrink(size64 m, size64 n) {
  return m > MIN_CAP && n + 1 < (m >> 1);
}

bool check_table_grow(size64 m, size64 n) {
  return n > ceil(m * LF);
}

bool check_table_shrink(size64 m, size64 n) {
  return m > MIN_CAP && n < (m >> 4);
}

size64 adjust_alist_size(size64 n) {
  if ( n == 0 || n > MAX_ARITY )
    return 0;

  n = ceil2(n);
  n = max(n, MIN_CAP);

  return n;
}

size64 adjust_buffer_size(size64 n) {
  if ( n == 0 || n >= MAX_ARITY )
    return 0;

  n = ceil2(n+1);
  n = max(n, MIN_CAP);

  return n;
}

size64 adjust_table_size(size64 n) {
  if ( n == 0 || n > MAX_ARITY )
    return 0;

  n = ceil2(ceil(n * LF_I));
  n = max(n, MIN_CAP);

  return n;
}


/* Initialization */

#undef MIN_CAP
#undef LF
#undef LF_I
