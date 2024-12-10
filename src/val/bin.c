#include <string.h>

#include "val/bin.h"
#include "val/type.h"
#include "val/func.h"

#include "lang/compare.h"

#include "vm/heap.h"

#include "util/collection.h"
#include "util/hash.h"
#include "util/memory.h"
#include "util/number.h"
#include "util/text.h"

/* C types */
/* Forward declarations */
// Helpers
static byte*  bytes_at(byte* d, size64 n, size64 es);
static Bin*   new_bin(CType ctype);
static void   init_bin(Bin* b, CType ctype);
static MBin*  new_mbin(CType ctype);
static void   init_mbin(MBin* b, CType ctype);
static size64 grow_mbin(MBin* b, size64 n);
static size64 shrink_mbin(MBin* b, size64 n);

// Interfaces
// Bin
void   free_bin(State* s, void* x);
void   clone_bin(State* s, void* x, bool d);
hash64 hash_bin(Val x);
bool   egal_bins(Val x, Val y);
int    order_bins(Val x, Val y);

// MBin
void free_mbin(State* s, void* x);
void clone_mbin(State* s, void* x, bool d);
void seal_mbin(State* s, void* x, bool d);

// External

/* Globals */
Func BinCtor = {
  .type    = &FuncType,
  .vtype   = T_FUNC,
  ._name   = "Bin"
};

Func MBinCtor = {
  .type    = &FuncType,
  .vtype   = T_FUNC,
  ._name   = "MBin"
};

VTable BinVTable = {
  .vtype  = T_BIN,
  .flags  = MF_SEALED,
  .dsize  = sizeof(Bin*),
  .osize  = sizeof(Bin),
  .tag    = OBJECT,
  .free   = free_bin,
  .clone  = clone_bin,
  .hash   = hash_bin,
  .egal   = egal_bins,
  .order  = order_bins
};

VTable MBinVTable = {
  .vtype  = T_MBIN,
  .dsize  = sizeof(MBin*),
  .osize  = sizeof(MBin),
  .tag    = OBJECT,
  .free   = free_mbin,
  .clone  = clone_mbin
};

Type BinType = {
  .type    = &TypeType,
  .vtype   = T_TYPE,
  ._name   = "Bin",
  .vtable  = &BinVTable,
  .ctor    = &BinCtor
};

Type MBinType = {
  .type    = &TypeType,
  .vtype   = T_TYPE,
  ._name   = "MBin",
  .vtable  = &MBinVTable,
  .ctor    = &MBinCtor
};

/* Helpers */
static byte* bytes_at(byte* b, size64 n, size64 es) {
  return b + n * es;
}

static Bin* new_bin(CType ctype) {
  Bin* out = new_obj(&Vm, &BinType, 0); init_bin(out, ctype);

  return out;
}

static void init_bin(Bin* bin, CType ctype) {
  bin->ctype  = ctype;
  bin->elsize = ct_size(ctype);
  bin->cnt    = 0;
  bin->bytes  = NULL;
}

static MBin* new_mbin(CType type) {
  MBin* b = new_obj(&Vm, &MBinType, 0);

  init_mbin(b, type);

  return b;
}

static void init_mbin(MBin* m, CType ctype) {
  // initialize array info
  m->ctype   = ctype;
  m->elsize  = ct_size(ctype);
  m->cnt     = 0;
  m->max     = 0;
  m->bytes   = NULL;
}

// TODO: maybe need some additional alist helpers?
static size64 grow_mbin(MBin* m, size64 n) {
  size64 o = m->cnt;

  if ( m->bytes == NULL )
    m->bytes = alloc_alist("rt/grow-mbin", NULL, n, m->elsize, &m->max);

  else
    m->bytes = realloc_alist("rt/grow-mbin", m->bytes, m->cnt, n, m->elsize, &m->max);

  m->cnt = n;

  return o;
}

static size64 shrink_mbin(MBin* m, size64 n) {
  m->bytes = realloc_alist("rt/shrink-mbin", m->bytes, m->cnt, n, m->elsize, &m->max);
  m->cnt   = n;

  return n;
}

/* Interfaces */
// Bin interfaces
void free_bin(State* s, void* x) {
  (void)s;

  Bin* b = x;

  s_free("rt/free", b->bytes);
}

void clone_bin(State* s, void* x, bool d) {
  (void)s;
  (void)d;
  
  Bin* b = x;

  b->bytes = s_cdup("rt/clone", b->bytes, b->cnt, b->elsize);
}

hash64 hash_bin(Val x) {
  Bin* b = as_bin(x);

  return hash_bytes(b->bytes, b->cnt*b->elsize);
}

bool egal_bins(Val x, Val y) {
  Bin* bx = as_bin(x), * by = as_bin(y);
  bool r  = bx->ctype == by->ctype && bx->cnt == by->cnt;

  if ( r )
    r = memcmp(bx->bytes, by->bytes, bx->cnt*bx->elsize) == 0;

  return r;
}

int order_bins(Val x, Val y) {
  Bin* bx = as_bin(x), * by = as_bin(y);
  int o;

  if ( bx->ctype != by->ctype )
    o = cmp(bx->ctype, by->ctype);

  else {
    size64 maxc = min(bx->cnt, by->cnt);
    o           = memcmp(bx->bytes, by->bytes, maxc*bx->elsize);

    if ( o == 0 )
      o = 0 - (bx->cnt < by->cnt) + (bx->cnt > by->cnt);
  }

  return o;
}

// MBin interfaces
void free_mbin(State* s, void* x) {
  (void)s;

  MBin* m = x;

  s_free("rt/free", m->bytes);
}

void clone_mbin(State* s, void* x, bool d) {
  (void)s;
  (void)d;

  MBin* m = x;

  m->bytes = s_cdup("rt/clone", m->bytes, m->max, m->elsize);
}

void seal_mbin(State* vm, void* x, bool d) {
  (void)vm;
  (void)d;

  MBin* m   = x;
  m->bytes  = trim_array("rt/seal", m->bytes, m->cnt, m->elsize, false);
  m->max    = m->cnt;
  m->nohash = true;
}

/* APIs */
// Bin API
Bin* mk_bin(CType ctype, byte* d, size64 n) {
  assert(d);

  Bin* r   = new_bin(ctype);
  r->cnt   = n;
  r->bytes = alloc_array("rt/bin", d, n, r->elsize);

  return r;
}

void* bin_ref(Bin* b, size64 n) {
  assert(n < b->cnt);

  return bytes_at(b->bytes, n, b->elsize);
}

Bin* bin_set(Bin* b, size64 n, void* d) {
  assert(n < b->cnt);
  assert(d);

  Bin* c   = clone_obj(&Vm, b, true);
  byte* l  = bytes_at(c->bytes, n, c->elsize);

  memcpy(l, d, c->elsize);

  return c;
}

Bin* bin_add(Bin* b, void* d) {
  assert(d);

  Bin* c    = clone_obj(&Vm, b, false);
  c->bytes  = pad_array("rt/bin-add", c->bytes, c->cnt, 1, c->elsize, true);
  byte* l   = bytes_at(c->bytes, c->cnt, c->elsize);
  c->cnt   += 1;
  c->hash   = 0; // invalidate hash

  memcpy(l, d, c->elsize);

  return c;
}

size64 bin_cpy(Bin* b, void* d, size64 n) {
  return copy_array(d, b->bytes, min(n, b->cnt), b->elsize);
}

// MBin API
MBin* mk_mbin(CType ctype) {
  return new_mbin(ctype);
}

void reset_mbin(MBin* b) {
  free_mbin(&Vm, b);

  b->cnt   = 0;
  b->max   = 0;
  b->bytes = NULL;
}

void* mbin_ref(MBin* b, size64 n) {
  assert(n < b->cnt);

  return bytes_at(b->bytes, n, b->elsize);
}

size64 mbin_add(MBin* b, void* d) {
  size64 off = b->cnt;

  if ( check_alist_grow(b->max, b->cnt+1) )
    grow_mbin(b, b->cnt+1);

  void* spc = bytes_at(b->bytes, off, b->elsize);

  memcpy(spc, d, b->elsize);

  return off;
}

size64 mbin_write(MBin* b, size64 n, byte* d) {
  size64 off = b->cnt;

  if ( check_alist_grow(b->max, b->cnt+n) )
    grow_mbin(b, b->cnt+n);

  if ( d ) {
    void* spc = bytes_at(b->bytes, off, b->elsize);
    memcpy(spc, d, n*b->elsize);
  }

  return off;
}

void mbin_set(MBin* b, size64 n, void* d) {
  assert(n < b->cnt);

  void* spc = bytes_at(b->bytes, n, b->elsize);

  memcpy(spc, d, b->elsize);
}

size64 mbin_cpy(MBin* b, void* d, size64 n) {
  return copy_array(d, b->bytes, min(n, b->cnt), b->elsize);
}

/* Initialization */
void rl_init_val_bin(void) {
  init_builtin_type(&Vm, &BinType);
  init_builtin_type(&Vm, &MBinType);
}
