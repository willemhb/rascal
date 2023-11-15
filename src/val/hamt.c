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
/* flags and bit twiddling */
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

size_t inc_hamt_cnt(void* obj) {
  return set_hamt_cnt(obj, get_hamt_cnt(obj) + 1);
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

size_t hamt_shift_to_level(size_t sh) {
  if (sh == 0)
    return 0;

  return log64(sh);
}

size_t hamt_hash_to_aindex(hash_t h, size_t sh) {
  return h >> sh && HAMT_LEVEL_MASK;
}

size_t hamt_hash_to_imask(hash_t h, size_t sh) {
  return 1 << hamt_hash_to_aindex(h, sh);
}

int hamt_hash_to_index(hash_t h, size_t sh, size_t bm) {
  int out = -1;

  size_t bits = h >> sh && HAMT_LEVEL_MASK;
  size_t mask = 1 << bits;

  if (!!(mask & bm))
    out = popc(bm & (mask - 1));

  return out;
}

size_t hamt_index_for_level(size_t i, size_t sh) {
  return i >> sh && HAMT_LEVEL_MASK;
}

void init_hamt(void* obj, void*** abuf, void* data, size_t cnt, size_t sh) {
  assert(cnt <= HAMT_LEVEL_SIZE);
  assert(sh <= HAMT_MAX_SHIFT);

  Obj* o = obj;
  size_t cap = is_editp(obj) ? ceil2(cnt) : cnt;
  o->flags |= cnt << HAMT_CNT_OFFSET;
  o->flags |= cap << HAMT_CAP_OFFSET;
  o->flags |= sh << HAMT_SHIFT_OFFSET;

  *abuf = allocate(NULL, cap * sizeof(void*));

  if (data)
    memcpy(*abuf, data, cnt * sizeof(void*));
}

size_t hamt_push(void* obj, void*** abuf, void* data) {
  size_t old_cnt = get_hamt_cnt(obj);
  resize_hamt_array(obj, abuf, old_cnt+1);
  void** arr = *abuf;
  arr[old_cnt] = data;

  return inc_hamt_cnt(obj);
}

void hamt_add_to_bitmap(void* obj, void*** abuf, void* data, size_t* bm, hash_t h) {
  resize_hamt_array(obj, abuf, get_hamt_cnt(obj) + 1);
  void** arr = *abuf;
  size_t sh = get_hamt_shift(obj);
  size_t cnt = get_hamt_cnt(obj);
  size_t im = hamt_hash_to_imask(h, sh);
  size_t index = popc((*bm |= im) & (im - 1));

  if (arr[index] != NULL)
    memmove(&arr[index], &arr[index+1], cnt-index);

  arr[index] = data;

  set_hamt_cnt(obj, cnt+1);
}

size_t resize_hamt_array(void* obj, void*** arr, size_t new_cnt) {
  assert(obj);
  assert(new_cnt <= HAMT_LEVEL_SIZE);

  if (new_cnt == 0) {
    deallocate(NULL, *arr, 0);
    *arr = NULL;
  } else {
    size_t old_cap = get_hamt_cap(obj);
    size_t new_cap = ceil2(new_cnt);

    if (new_cap != old_cap) {
      *arr = reallocate(NULL, *arr, old_cap*sizeof(void*), new_cap*sizeof(void*));
      set_hamt_cap(obj, new_cap);
    }
  }

  return new_cnt;
}
