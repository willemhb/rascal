#include "vm/memory.h"

#include "val/hamt.h"

/* globals */
#define HAMT_CNT_MASK     0x0000ff00u
#define HAMT_CAP_MASK     0x00ff0000u
#define HAMT_SHIFT_MASK   0xff000000u
#define HAMT_SIZE_MASK    0x000000ffu // 255
#define HAMT_CNT_OFFSET   0x00000008u // 008
#define HAMT_CAP_OFFSET   0x00000010u // 16
#define HAMT_SHIFT_OFFSET 0x00000018u // 24

/* External API */
bool get_hamt_editp(void* obj) {
  return get_mfl(obj, EDITP);
}

bool set_hamt_editp(void* obj) {
  return set_mfl(obj, EDITP);
}

size_t get_hamt_cnt(void* obj) {
  assert(obj);
  Obj* o = obj;
  return o->flags >> HAMT_CNT_OFFSET & HAMT_SIZE_MASK;
}

size_t set_hamt_cnt(void* obj, size_t n) {
  assert(obj);
  assert(n <= HAMT_LEVEL_SIZE);

  Obj* o = obj;
  o->flags &= ~HAMT_CNT_MASK;
  o->flags |= n << HAMT_CNT_OFFSET;

  return n;
}

size_t get_hamt_cap(void* obj) {
  assert(obj);
  Obj* o = obj;
  return o->flags >> HAMT_CAP_OFFSET & HAMT_SIZE_MASK;
}

size_t set_hamt_cap(void* obj, size_t n) {
  assert(obj);
  assert(n > 0 && n <= HAMT_LEVEL_SIZE);

  Obj* o = obj;
  o->flags &= ~HAMT_CAP_MASK;
  o->flags |= n << HAMT_CAP_OFFSET;

  return n;
}

size_t get_hamt_shift(void* obj) {
  assert(obj);

  Obj* o = obj;
  return o->flags >> HAMT_SHIFT_OFFSET & HAMT_SIZE_MASK;
}

size_t set_hamt_shift(void* obj, size_t n) {
  assert(obj);
  assert(n <= HAMT_MAX_SHIFT);

  Obj* o = obj;
  o->flags &= ~HAMT_SHIFT_MASK;
  o->flags |= n << HAMT_SHIFT_OFFSET;

  return n;
}

int hamt_hash_to_index(hash_t h, size_t sh, size_t bm) {
  int out = -1;

  size_t bits = h >> sh;
  size_t mask = 1 << bits;

  if (!!(mask & bm))
    out = popc(bm & (mask - 1));

  return out;
}

void init_hamt(void* obj, void** arr, void* data, size_t cnt, size_t sh, size_t es) {
  assert(obj);
  assert(arr);
  assert(cnt <= HAMT_LEVEL_SIZE);
  assert(sh <= HAMT_MAX_SHIFT);

  Obj* o = obj;
  size_t cap = ceil2(cnt);
  o->flags |= cnt << HAMT_CAP_OFFSET;
  o->flags |= cap << HAMT_CAP_OFFSET;
  o->flags |= sh << HAMT_SHIFT_OFFSET;

  *arr = allocate(NULL, cap * es);

  if (data)
    memcpy(*arr, data, cap * es);
}

size_t resize_hamt_array(void* obj, void** arr, size_t new_cnt, size_t es) {
  assert(obj);
  assert(new_cnt <= HAMT_LEVEL_SIZE);

  if (new_cnt == 0) {
    deallocate(NULL, *arr, 0);
    *arr = NULL;
  } else {
    size_t old_cap = get_hamt_cap(obj);
    size_t new_cap = ceil2(new_cnt);

    if (new_cap != old_cap) {
      *arr = reallocate(NULL, *arr, old_cap*es, new_cap*es);
      set_hamt_cap(obj, new_cap);
    }

    set_hamt_cnt(obj, new_cnt);
  }

  return get_hamt_cap(obj);
}
