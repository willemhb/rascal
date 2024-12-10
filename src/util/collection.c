#include <math.h>
#include <string.h>

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

static char* aerr_fmt =  "requested allocation of %zu elements exceeds runtime limit of %zu";

#define alloc_err(f, n) rl_runt_err(&Main, f, aerr_fmt, n, os)

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
  n = min(n, MAX_ARITY);

  return n;
}

size64 adjust_buffer_size(size64 n, bool ae) {
  if ( (n == 0 && !ae) || n >= MAX_ARITY )
    return 0;

  n = ceil2(n+1);

  n = max(n, MIN_CAP);
  n = min(n, MAX_ARITY);

  return n;
}

size64 adjust_table_size(size64 n) {
  if ( n == 0 || n > MAX_ARITY )
    return 0;

  n = ceil2(ceil(n * LF_I));
  n = max(n, MIN_CAP);
  n = min(n, MAX_ARITY);

  return n;
}

// alloc/realloc helpers
size64 copy_array(void* dst, void* src, size64 n, size64 os) {
  assert(n < MAX_ARITY);
  size64 size = n*os;
  memcpy(dst, src, n*os);
  return size;
}

void* alloc_array(char* fn, void* ini, size64 n, size64 os) {
  if ( n > MAX_ARITY )
    alloc_err(fn, n);

  void* array = s_calloc(fn, n, os, 0);

  if ( ini )
    copy_array(array, ini, n, os);

  return array; 
}

void* realloc_array(char* fn, void* arr, size64 on, size64 nn, size64 os) {
  if ( nn > MAX_ARITY )
    alloc_err(fn, nn);

  return s_crealloc(fn, arr, on, nn, os, 0);
}

void* alloc_string(char* fn, void* ini, size64 n, size64 os, bool ae) {
  if ( n+1 > MAX_ARITY )
    alloc_err(fn, n+1);

  void* string = s_salloc(fn, n, os, 0, ae);

  if ( ini )
    memcpy(string, ini, n*os);

  return string;
}

void* realloc_string(char* fn, void* str, size64 on, size64 nn, size64 os, bool ae) {
  if ( nn > MAX_ARITY )
    alloc_err(fn, nn);

  return s_srealloc(fn, str, on, nn, os, 0, ae);
}

void* alloc_alist(char* fn, void* ini, size64 n, size64 os, size64 *m) {
  if ( n > MAX_ARITY )
    alloc_err(fn, n);

  size64 p = adjust_alist_size(n);

  if ( m )
    *m = p;

  void* alist = s_calloc(fn, p, os, 0);

  if ( ini )
    memcpy(alist, ini, n*os);

  return alist;
}

void* realloc_alist(char* fn, void* arr, size64 on, size64 nn, size64 os, size64* m) {
  if ( nn > MAX_ARITY )
    alloc_err(fn, nn);

  size64 p = adjust_alist_size(nn);

  if ( m )
    *m = p;

  return s_crealloc(fn, arr, on, p, os, 0);
}

void* alloc_buffer(char* fn, void* ini, size64 n, size64 os, size64* m, bool ae) {
  if ( n+1 > MAX_ARITY )
    alloc_err(fn, n);

  size64 p = adjust_buffer_size(n, ae);

  if ( m )
    *m = p;

  void* b = s_salloc(fn, p, os, 0, ae);

  if ( b && ini )
    memcpy(b, ini, n*os);

  return b;
}

void* realloc_buffer(char* fn, void* buf, size64 on, size64 nn, size64 os, size64* m, bool ae) {
  if ( nn+1 > MAX_ARITY )
    alloc_err(fn, nn);

  size64 p = adjust_buffer_size(nn, ae);

  if ( m )
    *m = p;

  return s_srealloc(fn, buf, on, p, os, 0, ae);
}

void* alloc_table(char* fn, size64 n, size64 os, TInitFn init, size64* m) {
  if ( n > MAX_ARITY )
    alloc_err(fn, n);

  size64 p = adjust_table_size(n);

  if ( m )
    *m = p;

  void* kvs = s_calloc(fn, p, os, 0);

  if ( init )
    init(kvs, p);

  return kvs;
}

void* realloc_table(char* fn, void* kvs, size64 om, size64 nn, size64 os, TRehashFn rehash, size64 *m) {
  if ( nn > MAX_ARITY )
    alloc_err(fn, nn);

  size64 p = adjust_table_size(nn);

  if ( m )
    *m = p;

  void* nkvs = s_calloc(fn, p, os, 0);

  if ( rehash )
    rehash(kvs, om, nkvs, p);

  s_free(fn, kvs);

  return nkvs;
}

/* Initialization */

#undef MIN_CAP
#undef LF
#undef LF_I
#undef alloc_err
