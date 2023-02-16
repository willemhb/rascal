#ifndef utils_h
#define utils_h

#include <stdio.h>

#include "common.h"

/* C types */
typedef uint64 uhash;

/* API */
// table/alist helpers --------------------------------------------------------
usize pad_stack_size(usize count, usize cap);
usize pad_alist_size(usize count, usize cap);
usize pad_table_size(usize count, usize cap);

// numeric comparisons --------------------------------------------------------
int cmp_ints(int x, int y);
int cmp_reals(double x, double y);
int cmp_words(uint64 x, uint64 y);

// hashing --------------------------------------------------------------------
uhash hash_uint(uint64 x);
uhash hash_float(double x);
uhash hash_ptr(const void *x);
uhash hash_str(const char *xs);
uhash hash_mem(const ubyte *xs, usize n);
uhash mix_hashes(uhash hx, uhash hy);

// misc io --------------------------------------------------------------------
void newln(void);
void fnewln(FILE *ios);
int peekc(void);
int fpeekc(FILE *ios);

// convenience macros ---------------------------------------------------------
#define MAX(x, y)				\
  ({						\
    __auto_type _x = x;				\
    __auto_type _y = y;				\
    _x < _y ? _y : _x;				\
  })

#define MIN(x, y)				\
  ({						\
    __auto_type _x = x;				\
    __auto_type _y = y;				\
    _x > _y ? _y : _x;				\
  })


#endif
