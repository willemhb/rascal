#include <string.h>

#include "val/str.h"
#include "val/collection.h"
#include "val/seq.h"

#include "vm/state.h"
#include "vm/heap.h"
#include "vm/type.h"
#include "vm/env.h"

#include "util/str.h"
#include "util/hash.h"
#include "util/number.h"

/* Globals */
// standard streams
Port Ins = {
  .tag     = T_PORT,
  .sealed  = true,
  .nohash  = true,
  .nosweep = true,
  .isrl    = true,
  .ios     = NULL
};

Port Outs = {
  .tag     = T_PORT,
  .sealed  = true,
  .nohash  = true,
  .nosweep = true,
  .isrl    = true,
  .ios     = NULL
};

Port Errs = {
  .tag     = T_PORT,
  .sealed  = true,
  .nohash  = true,
  .nosweep = true,
  .isrl    = true,
  .ios     = NULL
};

/* Internal APIs */
/* Runtime APIs & interfaces */
// Str APIs
// lifetime
void free_str(State* vm, void* x) {
  Str* s = x;

  // free characters
  rl_dealloc(NULL, s->cs, 0);

  if ( s->cached ) { // mark as tombstone to be cleaned up and temporarily prevent sweeping
    s->nosweep = true;
    vm->strs->nts++;
  }
}

// comparison
bool egal_strs(Val x, Val y) {
  Str* sx = as_str(x), * sy = as_str(y);

  return egal_str_obs(sx, sy);
}

int order_strs(Val x, Val y) {
  Str* sx = as_str(x), * sy = as_str(y);

  return order_str_obs(sx, sy);
}

// sequence
bool str_empty(void* x) {
  Str* s = x;

  return s->cnt == 0;
}

// Buffer
// lifetime
void free_buffer(State* vm, void* x) {
  (void)vm;

  Buffer* b = x;

  rl_dealloc(NULL, b->data, 0);

  init_buffer(vm, b, VOID);
}

/* External APIs */
// Port APIs
int rl_printf(Port* p, char* fmt, ...) {
  assert(p->ios);
  va_list va;
  va_start(va, fmt);
  int r = vfprintf(p->ios, fmt, va);
  va_end(va);
  return r;
}

int rl_printv(Port* p, char* fmt, va_list va) {
  assert(p->ios);
  int r = vfprintf(p->ios, fmt, va);
  return r;
}

int rl_putc(Port* p, Glyph g) {
  assert(p->ios);
  return fputc(g, p->ios);
}

int rl_getc(Port* p) {
  assert(p->ios);
  return fgetc(p->ios);
}

int rl_ungetc(Port* p, Glyph g) {
  assert(p->ios);
  return ungetc(g, p->ios);
}

int rl_peekc(Port* p) {
  int c = rl_getc(p);

  if ( c != EOF )
    rl_ungetc(p, c);

  return c;
}

// Str APIs
Str* new_str(char* cs, size64 n, bool i, hash64 h) {
  Str* s    = new_obj(&Vm, T_STR, MF_SEALED);

  // initialize string fields
  s->hash   = mix_hashes(Vm.vts[T_STR]->hash, h);
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

bool egal_str_obs(Str* sx, Str* sy) {
  return sx->cnt == sy->cnt && sx->chash == sy->chash && seq(sx->cs, sy->cs);
}

int order_str_obs(Str* sx, Str* sy) {
  return scmp(sx->cs, sy->cs);
}

size64 str_buf(Str* s, char* buf, size64 bufsz) {
  size64 nwrite = min(s->cnt, bufsz-1);

  strncpy(buf, s->cs, nwrite);

  return nwrite;
}

// Buffer APIs

// mostly internal methods
