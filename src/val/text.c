#include <string.h>

#include "val/text.h"
#include "val/table.h"
#include "val/sequence.h"

#include "vm/state.h"
#include "vm/heap.h"
#include "vm/type.h"
#include "vm/environ.h"

#include "util/text.h"
#include "util/hash.h"
#include "util/number.h"

/* Globals */
/* Internal APIs */
static void* buffer_offset(Buffer* b, size64 n) {
  return (void*)&b->data[n*b->elsize];
}

static bool check_buffer_grow(size64 n, size64 c, bool e) {
  return n + e > c;
}

static bool check_buffer_shrink(size64 n, size64 c, bool e) {
  return c == MIN_ARR ? n == 0 : (n + e) < c >> 1;
}

static size64 calc_buffer_size(size64 n, bool e) {
  if ( n == 0 )
    return 0;
  
  size64 p = ceil2(n + e);

  return max(p, MIN_ARR);
}

static byte* alloc_buffer_bin(size64 n, size64 es) {
  return rl_alloc(NULL, n*es);
}

static byte* realloc_buffer_bin(byte* b, size64 on, size64 nn, size64 es) {
  return rl_realloc(NULL, b, on*es, nn*es);
}

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

void str_sinit(Seq* s) {
  Str* cs = s->src;

  s->done = cs->cnt == 0;
}

Val str_sfirst(Seq* s) {
  Str* cs = s->src;

  return tag(cs->cs[s->off]);
}

void str_srest(Seq* s) {
  Str* cs = s->src;

  s->done = s->off == cs->cnt;

  if ( !s->done )
    s->off++;
}

// print
size64 pr_str(State* vm, Port* p, Val x) {
  (void)vm;

  Str* xs  = as_str(x);
  size64 r = rl_printf(p, "\"%s\"", xs->cs);

  return r;
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

// Str APIs
Str* new_str(char* cs, size64 n, bool i, hash64 h) {
  Str* s    = new_obj(&Vm, T_STR, MF_SEALED);

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
Buffer* new_buffer(CType type) {
  Buffer* b = new_obj(&Vm, T_BUFFER, 0);

  init_buffer(&Vm, b, type);

  return b;
}

void init_buffer(State* vm, Buffer* b, CType type) {
  (void)vm;

  // initialize Ctype info (pass VOID to indicate that this field doesn't need to be set)
  if ( type ) {
    b->type    = type;
    b->elsize  = ct_size(type);
    b->encoded = ct_is_encoded(type);
  }

  // initialize array info
  b->cnt     = 0;
  b->cap     = 0;
  b->data     = NULL;
}

void grow_buffer(Buffer* b, size64 n) {
  size64 newc = calc_buffer_size(n, b->encoded);
  size64 oldc = b->cap;
  byte*  newd = realloc_buffer_bin(b->data, oldc, newc, b->encoded);
  b->cap      = newc;
  b->data     = newd;
}

void shrink_buffer(Buffer* b, size64 n) {
  if ( n == 0 )
    free_buffer(&Vm, b);

  else {
    size64 newc = calc_buffer_size(n, b->encoded);
    byte*  newb = realloc_buffer_bin(b->data, b->cap, newc, b->encoded);
    b->cap      = newc;
    b->data     = newb;
  }
}

void resize_buffer(Buffer* b, size64 n) {
  if ( check_buffer_shrink(n, b->cap, b->encoded) )
    shrink_buffer(b, n);

  else if ( check_buffer_grow(n, b->cap, b->encoded) )
    grow_buffer(b, n);
}

// external methods
void* buffer_ref(Buffer* b, size64 n) {
  assert(n < b->cnt);

  return buffer_offset(b, n);
}

size64 buffer_add(Buffer* b, word_t d) {
  if ( check_buffer_grow(b->cnt+1, b->cap, b->encoded) )
    grow_buffer(b, b->cnt+1);

  void* spc = buffer_offset(b, b->cnt);

  switch ( b->elsize ) {
    case 1: *(uint8*)spc  = (uint8)d;  break;
    case 2: *(uint16*)spc = (uint16)d; break;
    case 4: *(uint32*)spc = (uint32)d; break;
    case 8: *(uint64*)spc = (uint64)d; break;
  }

  return b->cnt++;
}

size64 buffer_wrt(Buffer* b, size64 n, byte* d) {
  if ( check_buffer_grow(b->cnt+n, b->cap, b->encoded) )
    grow_buffer(b, b->cnt+n);


  size64 o = b->cnt;
  
  if ( d ) {
    void* spc = buffer_offset(b, o);
    memcpy(spc, d, n*b->elsize);
  }

  b->cnt += n;

  return o;
}

void buffer_set(Buffer* b, size64 n, word_t d) {
  assert(n < b->cnt);

  void* spc = buffer_offset(b, n);

  switch ( b->elsize ) {
    case 1: *(uint8*)spc  = (uint8)d;  break;
    case 2: *(uint16*)spc = (uint16)d; break;
    case 4: *(uint32*)spc = (uint32)d; break;
    case 8: *(uint64*)spc = (uint64)d; break;
  }
}
