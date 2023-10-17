#include "util/number.h"
#include "util/collection.h"

#include "memory.h"

static const size_t minStackCap = 8;
static const size_t minOrdsCap  = 8;
static const double loadFactor  = 0.625; // 5/8
static const double loadFactorC = 1.6;   // 8/5

/* general collection utilities */
bool check_alist_resize(size_t new_cnt, size_t old_cap, bool encoded) {
  size_t padded = new_cnt + encoded;
  return padded > old_cap || padded < (old_cap >> 1);
}

bool check_table_size(size_t new_cnt, size_t old_cap) {
  size_t padded = loadFactorC * new_cnt;

  return padded > old_cap || padded < (old_cap >> 1);
}

size_t pad_alist_size(size_t old_cnt, size_t new_cnt, size_t old_cap, bool encoded) {
  /* CPython resize algorithm */
  size_t out;

  if (new_cnt == 0)
    out = 0;

  else if (check_alist_resize(new_cnt, old_cap, encoded)) {
    size_t new_cap = (new_cnt + (new_cnt >> 3) + 6) & ~(size_t)3;

    if (new_cnt - old_cnt > new_cap - new_cnt)
      new_cap = (new_cnt + 3) & ~(size_t)3;

    out = new_cap;
  } else {
    out = old_cap;
  }

  return out;
}

size_t pad_stack_size(size_t old_cnt, size_t new_cnt, size_t old_cap, bool encoded) {
  (void)old_cnt;

  size_t out;

  if (new_cnt == 0)
    out = 0;

  else if (check_alist_resize(new_cnt, old_cap, encoded))
    out = max(minStackCap, ceil_2(new_cnt));

  else
    out = max(minStackCap, ceil_2(new_cnt));

  return out;
}

size_t pad_ord_size(size_t old_cnt, size_t new_cnt, size_t old_cap) {
  (void)old_cnt;

  size_t out;

  if (new_cnt == 0)
    out = 0;

  else if (check_table_resize(new_cnt, old_cap))
    out = max(minOrdsCap, ceil_2(new_cnt));

  else
    out = old_cap;

  return out;
}

size_t get_ord_size(size_t cnt) {
  if (cnt <= INT8_MAX)
    return sizeof(int8_t);

  else if (cnt <= INT16_MAX)
    return sizeof(int16_t);

  else if (cnt <= INT32_MAX)
    return sizeof(int32_t);

  else
    return sizeof(int64_t);
}

void* alloc_ords(size_t cap, size_t o_size) {
  if (o_size == 0)
    o_size = get_ord_size(cap);

  size_t m_size = cap*o_size;
  void* out     = allocate(NULL, m_size);

  memset(out, -1, m_size);
  return out;
}
