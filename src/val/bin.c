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
static byte* bytes_at(byte* d, size64 n, size64 es);
static Bin*  new_bin(CType ctype);
static void  init_bin(Bin* b, CType ctype);

// Interfaces
// Bin
void   free_bin(State* s, void* x);
void   clone_bin(State* s, void* x);
hash64 hash_bin(Val x);
bool   egal_bins(Val x, Val y);
int    order_bins(Val x, Val y);

// MBin
void free_mbin(State* s, void* x);
void clone_mbin(State* s, void* x);
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
  .flags  = 0,
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

/* Interfaces */
// Bin interfaces
void free_bin(State* s, void* x) {
  (void)s;

  Bin* b = x;

  s_free("rt/free", b->bytes);
}

void clone_bin(State* s, void* x) {
  (void)s;
  
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

void clone_mbin(State* s, void* x) {
  (void)s;

  MBin* m = x;

  m->bytes = s_cdup("rt/clone", m->bytes, m->max, m->elsize);
}

void seal_mbin(State* vm, void* x, bool d) {
  (void)vm;
  (void)d;

  MBin* m   = x;
  m->nohash = true;
  m->bytes  = trim_array("rt/seal", m->bytes, m->cnt, m->elsize);
  m->max    = m->cnt;
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

  
}

// MBin API
MBin* new_mbin(CType type) {
  MBin* b = new_obj(&Vm, &MBinType, 0);

  init_mbin(&Vm, b, type);

  return b;
}

void init_mbin(State* vm, MBin* b, CType type) {
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

void grow_mbin(MBin* b, size64 n) {
  size64 newc = calc_mbin_size(n, b->encoded);
  size64 oldc = b->cap;
  byte*  newd = realloc_mbin_bin(b->data, oldc, newc, b->encoded);
  b->cap      = newc;
  b->data     = newd;
}

void shrink_mbin(MBin* b, size64 n) {
  if ( n == 0 )
    free_mbin(&Vm, b);

  else {
    size64 newc = calc_mbin_size(n, b->encoded);
    byte*  newb = realloc_mbin_bin(b->data, b->cap, newc, b->encoded);
    b->cap      = newc;
    b->data     = newb;
  }
}

void resize_mbin(MBin* b, size64 n) {
  if ( check_mbin_shrink(n, b->cap, b->encoded) )
    shrink_mbin(b, n);

  else if ( check_mbin_grow(n, b->cap, b->encoded) )
    grow_mbin(b, n);
}

// external methods
void* MBin_ref(MBin* b, size64 n) {
  assert(n < b->cnt);

  return MBin_offset(b, n);
}

size64 MBin_add(MBin* b, word_t d) {
  if ( check_mbin_grow(b->cnt+1, b->cap, b->encoded) )
    grow_mbin(b, b->cnt+1);

  void* spc = MBin_offset(b, b->cnt);

  switch ( b->elsize ) {
    case 1: *(uint8*)spc  = (uint8)d;  break;
    case 2: *(uint16*)spc = (uint16)d; break;
    case 4: *(uint32*)spc = (uint32)d; break;
    case 8: *(uint64*)spc = (uint64)d; break;
  }

  return b->cnt++;
}

size64 MBin_wrt(MBin* b, size64 n, byte* d) {
  if ( check_mbin_grow(b->cnt+n, b->cap, b->encoded) )
    grow_mbin(b, b->cnt+n);


  size64 o = b->cnt;
  
  if ( d ) {
    void* spc = mbin_offset(b, o);
    memcpy(spc, d, n*b->elsize);
  }

  b->cnt += n;

  return o;
}

void MBin_set(MBin* b, size64 n, word_t d) {
  assert(n < b->cnt);

  void* spc = mbin_offset(b, n);

  switch ( b->elsize ) {
    case 1: *(uint8*)spc  = (uint8)d;  break;
    case 2: *(uint16*)spc = (uint16)d; break;
    case 4: *(uint32*)spc = (uint32)d; break;
    case 8: *(uint64*)spc = (uint64)d; break;
  }
}

/* Initialization */
