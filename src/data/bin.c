#include <string.h>
#include <assert.h>

#include "base/type.h"

#include "data/bin.h"
#include "data/stream.h"
#include "data/table.h"
#include "data/sym.h"
#include "data/func.h"
#include "data/int.h"

#include "runtime/memory.h"
#include "runtime/error.h"

#include "util/collections.h"
#include "util/hash.h"

/* globals */
Val MutOpt, ImmutOpt, U8Opt;

Table CtypeMap;

/* local helpers */
static uhash hash_bin_data(Ctype type, usize arity, void* data) {
  uhash cthash = hash_uint(type);
  uhash bthash = hash_uint(BIN_TYPE);
  uhash dhash  = hash_mem(data, arity*ctype_size(type));
  uhash dbthash= mix_hashes(bthash, dhash);

  return mix_hashes(cthash, dbthash);
}

static char* ctype_name(Bin* b, bool abbrev) {
  return (abbrev ? ShortCtypeNames : LongCtypeNames)[bin_ctype(b)];
}

static void print_u8(void* spc, Stream* ios) {
  sfmt(ios, "%.3u", *(uint8*)spc);
}

static void print_u16(void* spc, Stream* ios) {
  sfmt(ios, "%.5u", *(uint16*)spc);
}

static void print_u32(void* spc, Stream* ios) {
  sfmt(ios, "%.8xu", *(uint32*)spc);
}

static void print_u64(void* spc, Stream* ios) {
  sfmt(ios, "%.16xu", *(uint64*)spc);
}

static void (*get_printer(Bin* b))(void* spc, Stream* ios) {
  switch (bin_el_size(b)) {
    case 1: return print_u8;
    case 2: return print_u16;
    case 4: return print_u32;
    case 8: return print_u64;
  }

  unreachable();
}

/* API */
bool is_bin(Val x) {
  return has_type(x, BIN_TYPE);
}

Bin* as_bin(Val x) {
  assert(is_bin(x));

  return ((ValData)VAL_BITS(x)).as_bin;
}

Val mk_bin(Bin* b) {
  assert(b != NULL);

  return mk_obj((Obj*)b);
}

// metadata -------------------------------------------------------------------
Ctype bin_ctype(Bin* b) {
  return get_flags((Obj*)b, 0x1f);
}

bool is_string(Bin* b) {
  return has_wflag((Obj*)b, ENCODED|FROZEN, ENCODED|FROZEN);
}

usize bin_el_size(Bin* b) {
  return ctype_size(bin_ctype(b));
}

usize bin_arr_size(Bin* b) {
  return b->count * bin_el_size(b);
}

usize bin_used_size(Bin* b) {
  usize total = b->count + has_flag((Obj*)b, ENCODED);
  return total * bin_el_size(b);
}

usize bin_alloc_size(Bin* b) {
  return b->cap * bin_el_size(b);
}

// constructors & memory management -------------------------------------------
Bin* new_bin(Ctype type, usize n, void* data, bool mutp) {
  Bin* out = allocate(sizeof(Bin));
  init_bin(out, type, n, data, mutp);
  return out;
}

void init_bin(Bin* self, Ctype type, usize n, void* data, bool mutp) {
  bool encp = is_encoding(type);
  int fl = mutp*FROZEN | type | encp*ENCODED;

  init_obj((Obj*)self, BIN_TYPE, fl);

  resize_bin(self, n);

  if (data)
    memcpy(self->array, data, bin_arr_size(self));
}

void resize_bin(Bin* self, usize n) {
  bool encp = has_flag((Obj*)self, ENCODED);
  usize cap = pad_alist_size(n + encp, self->cap);

  if (cap != self->cap) {
    self->array = reallocate(self->array, cap * bin_el_size(self), bin_alloc_size(self));
    self->cap   = cap;
  }

  self->count = n;
}

// accessors & mutators -------------------------------------------------------
void* bin_ref(Bin* self, int i) {
  if (i < 0)
    i += self->count;

  assert(i >= 0 && (usize)i < self->count);
  return self->array + i * bin_el_size(self);
}

void* bin_set(Bin* self, int i, void* data) {
  void* spc = bin_ref(self, i);

  switch (bin_el_size(self)) {
    case 1:  *(uint8*)spc  = *(uint8*)data; break;
    case 2:  *(uint16*)spc = *(uint16*)data; break;
    case 4:  *(uint32*)spc = *(uint32*)data; break;
    case 8:  *(uint64*)spc = *(uint64*)data; break;
    default: unreachable();
  }

  return data;
}

usize bin_write(Bin* self, usize n, void* data) {
  usize off = self->count;
  
  resize_bin(self, off+n);
  
  void* spc = bin_ref(self, off);
  
  memcpy(spc, data, n * bin_el_size(self));

  return self->count;
}

// lifetime methods -----------------------------------------------------------
usize destruct_bin(void* self) {
  Bin* b = self;
  usize tofree = 0;
  
  if (!has_wflag((Obj*)b, NOFREE|ENCODED, NOFREE|ENCODED)) {
    tofree = bin_alloc_size(b);
    deallocate(b->array, tofree);
  }

  return tofree;
}

// io methods -----------------------------------------------------------------
 #include "lang/print.h"

void print_bin(Val x, void* state) {
  Print* p = state;
  Bin* b   = as_bin(x);

  if (is_string(b)) {
    sfmt(p->ios, "\"%s\"", b->array);
  } else {
    usize elsize = bin_el_size(b);
    void *spc = b->array;
    void (*printer)(void* spc, Stream* ios) = get_printer(b);

    sfmt(p->ios, "%s<", ctype_name(b, true));

    for (usize i=0; i < b->count; i++, spc+=elsize) {
      printer(spc, p->ios);

      if (i+1 < b->count)
	sfmt(p->ios, " ");
    }

    sfmt(p->ios, ">");
  }
}

// hash & comparison methods --------------------------------------------------
#include "lang/hash.h"
#include "util/string.h"
#include "util/number.h"

uhash hash_bin(Val x, void* state) {
  (void)state;

  Bin* b = as_bin(x);

  return hash_bin_data(bin_ctype(b), b->count, b->array);
}

bool equal_bins(Val x, Val y, void* state) {
  (void)state;

  Bin* xb = as_bin(x), * yb = as_bin(y);
  Ctype xt = bin_ctype(xb), yt = bin_ctype(yb);

  if (xt != yt)
    return false;

  if (xb->count != yb->count)
    return false;

  if (is_frozen((Obj*)xb) != is_frozen((Obj*)yb))
    return false;

  switch (ctype_size(xt)) {
    case 1: return u8cmp(xb->array, yb->array, xb->count) == 0;
    case 2: return u16cmp(xb->array, yb->array, xb->count) == 0;
    case 4: return u32cmp(xb->array, yb->array, xb->count) == 0;
    case 8: return u64cmp(xb->array, yb->array, xb->count) == 0;
  }

  unreachable();
}

int compare_bins(Val x, Val y, void* state) {
  (void)state;

  Bin* xb = as_bin(x), * yb = as_bin(y);
  Ctype xt = bin_ctype(xb), yt = bin_ctype(yb);

  if (xt != yt)
    return cmp_ints(xt, yt);

  usize max_cmp = MIN(xb->count, yb->count);
  int o;

  switch (ctype_size(xt)) {
    case 1:
      if ((o=u8cmp(xb->array, yb->array, max_cmp)))
	  return o;
      break;

    case 2:
      if ((o=u16cmp(xb->array, yb->array, max_cmp)))
	return o;
      break;

    case 4:
      if ((o=u32cmp(xb->array, yb->array, max_cmp)))
	return o;
      break;

    case 8:
      if ((o=u64cmp(xb->array, yb->array, max_cmp)))
	return o;
      break;
  }

  return cmp_words(xb->count, yb->count);
}

// native functions -----------------------------------------------------------
Val native_bin(int nargs, Val* args, Table* opts) {
  bool mutable = true; Ctype type = VOID;

  if (table_has(opts, ImmutOpt))
    mutable = false;

  Val* table = opts->table;

  for (int i=0; i<opts->count*2; i+= 2) {
    if (table[i] == MutOpt || table[i] == ImmutOpt)
      continue;

    Val result = table_ref(&CtypeMap, table[i]);

    if (result != NOTFOUND_VAL) {
      type = as_int(result);
      break;
    }
  }

  
}

Val native_bin_ref(int nargs, Val* args, Table* opts);
Val native_bin_set(int nargs, Val* args, Table* opts);
Val native_bin_add(int nargs, Val* args, Table* opts);
Val native_bin_write(int nargs, Val* args, Table* opts);
  bool mutable = true; Ctype type = VOID;

  if (table_has(opts, ImmutOpt))
    mutable = false;

  Val* table = opts->table;

  for (int i=0; i<opts->count*2; i+= 2) {
    if (table[i] == MutOpt || table[i] == ImmutOpt)
      continue;

    Val result = table_ref(&CtypeMap, table[i]);

    if (result != NOTFOUND_VAL) {
      type = as_int(result);
      break;
    }
  }


// initialization -------------------------------------------------------------
void bin_init(void) {
  // initialize type ----------------------------------------------------------
  MetaTables[BIN_TYPE] = (Mtable) {
    "bin",
    BIN_TYPE,
    DATA,
    sizeof(Bin),

    NULL,
    destruct_bin,
    print_bin,
    hash_bin,
    equal_bins,
    compare_bins
  };
  bool mutable = true; Ctype type = VOID;

  if (table_has(opts, ImmutOpt))
    mutable = false;

  Val* table = opts->table;

  for (int i=0; i<opts->count*2; i+= 2) {
    if (table[i] == MutOpt || table[i] == ImmutOpt)
      continue;

    Val result = table_ref(&CtypeMap, table[i]);

    if (result != NOTFOUND_VAL) {
      type = as_int(result);
      break;
    }
  }

  // initialize default options -----------------------------------------------
  MutOpt   = symbol(":mutable");
  ImmutOpt = symbol(":immutable");
  U8Opt    = symbol(":uint8");

  deftype("bin", 0, NATIVE|VARGS|VOPTS, native_bin, &MetaTables[BIN_TYPE], MutOpt, U8Opt);

  // initialize Ctype mapping & keywords --------------------------------------
  init_table(&CtypeMap, 0, NULL, NULL, NOFREE);

  for (int i=VOID; i <NUM_CTYPES; i++) {
    table_set(&CtypeMap, keyword(ShortCtypeNames[i]), mk_int(i));
    table_set(&CtypeMap, keyword(LongCtypeNames[i]), mk_int(i));
  }
}
