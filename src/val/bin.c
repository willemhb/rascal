#include "val/bin.h"
#include "val/type.h"
#include "val/func.h"

#include "vm/heap.h"

#include "util/number.h"

/* C types */

/* Forward declarations */
// Helpers
static void* mbin_offset(MBin* b, size64 n);
static byte* alloc_mbin_bin(MBin* b, size64 n);
static byte* realloc_mbin_bin(MBin* b, size64 n);

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

// External

/* Globals */
Func BinCtor = {
  .type    = &FuncType,
  .vtype   = T_FUNC,
  .nosweep = true, 
  ._name  = "Bin"
};

VTable BinVTable = {
  .vtype  = T_BIN,
  .dsize  = sizeof(Bin*),
  .obsize = sizeof(Bin),
  .tag    = OBJECT,
  .free   = free_bin,
  .clone  = clone_bin,
  .hash   = hash_bin,
  .egal   = egal_bins,
  .order  = order_bins
};

Type BinType = {
  .type    = &TypeType,
  .vtype   = T_TYPE,
  .nosweep = true,
  ._name   = "Bin",
  .vtable  = &BinVTable,
  .ctor    = &BinCtor,
};

Type MBinType = {

};

/* Helpers */
static void* mbin_offset(MBin* b, size64 n) {
  return (void*)&b->bytes[n*b->elsize];
}

static byte* alloc_mbin_bin(MBin* b, size64 n) {
  return rl_alloc(NULL, n*b->elsize);
}

static byte* realloc_mbin_bin(MBin* b, size64 on, size64 nn, size64 es) {
  return rl_realloc(NULL, b, on*es, nn*es);
}

/* Interfaces */

/* API */
MBin* new_mbin(CType type) {
  MBin* b = new_obj(&Vm, &MBinType, 0);

  init_mbin(&Vm, b, type);

  return b;
}

void init_MBin(State* vm, MBin* b, CType type) {
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

void grow_MBin(MBin* b, size64 n) {
  size64 newc = calc_MBin_size(n, b->encoded);
  size64 oldc = b->cap;
  byte*  newd = realloc_MBin_bin(b->data, oldc, newc, b->encoded);
  b->cap      = newc;
  b->data     = newd;
}

void shrink_MBin(MBin* b, size64 n) {
  if ( n == 0 )
    free_MBin(&Vm, b);

  else {
    size64 newc = calc_MBin_size(n, b->encoded);
    byte*  newb = realloc_MBin_bin(b->data, b->cap, newc, b->encoded);
    b->cap      = newc;
    b->data     = newb;
  }
}

void resize_MBin(MBin* b, size64 n) {
  if ( check_MBin_shrink(n, b->cap, b->encoded) )
    shrink_MBin(b, n);

  else if ( check_MBin_grow(n, b->cap, b->encoded) )
    grow_MBin(b, n);
}

// external methods
void* MBin_ref(MBin* b, size64 n) {
  assert(n < b->cnt);

  return MBin_offset(b, n);
}

size64 MBin_add(MBin* b, word_t d) {
  if ( check_MBin_grow(b->cnt+1, b->cap, b->encoded) )
    grow_MBin(b, b->cnt+1);

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
  if ( check_MBin_grow(b->cnt+n, b->cap, b->encoded) )
    grow_MBin(b, b->cnt+n);


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
