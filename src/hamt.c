#include <string.h>

#include "hamt.h"
#include "metaobject.h"
#include "object.h"
#include "number.h"

#include "memory.h"

// misc -----------------------------------------------------------------------
// globals --------------------------------------------------------------------
#define SHIFT      0x06ul
#define MAXN       0x40ul
#define MAXS       0x2aul
#define MINN       0x01ul
#define MASK       0x3ful
#define MAXH       0x07ul
#define MAXL       0x08ul
#define KEY(a, o)  ((a)[((o) & MASK) << 1])
#define VAL(a, o)  ((a)[(((o) & MASK) << 1) | 1])

// utilities ------------------------------------------------------------------
static uint32 tailoff(void* ptr, uint64 n) {
  hamt_t* hamt = ptr;

  if (hamt->arity < MAXN)
    return 0;

  return n & ~MASK;
}

static inline uint64 get_shift(uint32 shift, uint64 i) {
  return i >> shift & MASK;
}

static inline uint64 get_mask(uint32 shift, uint64 i) {
  return 1ul << get_shift(shift, i);
}

static inline uint64 get_index(uint32 shift, uint64 i, uint64 bitmap) {
  return popcnt(bitmap & (get_mask(shift, i) - 1));
}

static inline bool has_entry(uint32 shift, uint64 i, uint64 bitmap) {
  return !!(get_mask(shift, i) & bitmap);
}

static value_t* array_for(void* ptr, uint64 n) {
  hamt_root_t* hamt = ptr;

  if (n >= tailoff(hamt, n))
    return hamt->hamt.tail;

  tuple_t* root = hamt->root;

  for (uint32 shift=hamt->hamt.shift; shift > 0; shift -= SHIFT)
    root = as_tuple(root->slots[get_shift(shift, n)]);

  return root->slots;
}

static value_t* entry_for(void* ptr, uint64 n) {
  
}

// APIs -----------------------------------------------------------------------
// vector ---------------------------------------------------------------------
value_t vector_get(vector_t* self, uint64 n) {
  assert(n < self->root.hamt.arity);

  value_t* array = array_for(self, n);

  return array[n & MASK];
}

vector_t* vector_set(vector_t* self, uint64 n, value_t x) {
  
}

// dict -----------------------------------------------------------------------
value_t dict_get(dict_t* self, value_t key) {
  if (self == &EmptyDict)
    return NOTFOUND;

  if (key == NUL)
    return self->root.hamt.hasNul ? self->nulval : NOTFOUND;

  uint64 h = rl_hash(key);

  stencil_t* map = self->map;

  for (;;) {
    uint32 shift  = map->hamt.shift;
    uint64 bitmap = map->hamt.bitmap;
    value_t* tail = map->hamt.tail;
    
    if (has_entry(shift, h, bitmap)) {
      uint64 i  = get_index(shift, h, bitmap);
      value_t x = tail[i];

      if (is_object(x))
        map = (stencil_t*)as_object(x);

      else {
        uint64 n = as_fixnum(x);
        value_t* array = array_for(self, n);

        else
          return NOTFOUND;
      }
    }

    else
      return NOTFOUND;
  }
}

dict_t* dict_set(dict_t* self, value_t key, value_t val) {
  
}

#undef SHIFT
#undef MAXN
#undef MAXS
#undef MINN
#undef MASK
#undef MAXH
#undef MAXL
