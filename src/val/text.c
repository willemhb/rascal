#include <string.h>

#include "val/text.h"
#include "val/table.h"

#include "vm/state.h"
#include "vm/heap.h"
#include "vm/type.h"
#include "vm/environ.h"

#include "util/text.h"
#include "util/hash.h"
#include "util/number.h"

/* Globals */
/* Internal APIs */
void free_str(State* vm, void* x) {
  Str* s = x;

  // free characters
  rl_dealloc(NULL, s->cs, 0);

  if ( s->cached ) { // mark as tombstone to be cleaned up and temporarily prevent sweeping
    s->nosweep = true;
    vm->strs->nts++;
  }
}

bool egal_strs(Val x, Val y) {
  Str* sx = as_str(x), * sy = as_str(y);

  return sx->cnt == sy->cnt && sx->chash == sy->chash && seq(sx->cs, sy->cs);
}

int order_strs(Val x, Val y) {
  Str* sx = as_str(x), * sy = as_str(y);

  return scmp(sx->cs, sy->cs);
}

/* External APIs */
// String API

Str* new_str(char* cs, size64 n, bool i, hash64 h) {
  Str* s    = new_obj(&Vm, T_STR, MF_PERSISTENT);

  // initialize string fields
  s->hash   = mix_hashes(h, Vm.vts[T_STR].hash);
  s->cached = i;
  s->chash  = h;
  s->cnt    = n;
  s->cs     = rl_dup(&Vm, cs, n+1);

  // return string
  return s;
}

Str* mk_str(char* cs, size64 n, bool i) {
  Str* s;

  if ( i && n <= MAX_INTERN ) {
    /* call gc pre-emptively in case allocation would cause a collection and possible
       reallocation of the string table */
    rl_gc(&Vm, sizeof(Str));

    s = intern_str(Vm.strs, cs, n);
  } else {
    s = new_str(cs, n, false, hash_chars(cs, n));
  }

  return s;
}

Str* get_str(char* cs, size64 n) {
  return mk_str(cs, n, true);
}

Glyph str_ref(Str* s, size64 n) {
  // unsafe method, calls to str_ref from user code should be validated
  assert(n < s->cnt);

  return s->cs[n];
}

Str* str_set(Str* s, size64 n, Glyph g) {
  //unsafe method, calls to str_set from user code should be validated
  assert(n < s->cnt);

  // create buffer for modified string
  char buf[s->cnt+1];
  
  str_buf(s, buf, s->cnt+1);

  // update buffer with new character
  buf[n] = g;

  // get a copy of the modified string
  s = get_str(buf, s->cnt+1);
  
  return s;
}

size64 str_buf(Str* s, char* buf, size64 bufsz) {
  size64 nwrite = min(s->cnt, bufsz-1);

  strncpy(buf, s->cs, nwrite);

  return nwrite;
}
