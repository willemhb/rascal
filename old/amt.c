#include <string.h>

#include "util/amt.h"

/* C types */

/* Forward declarations */
// Helpers
// Interfaces
// External

/* Globals */

/* Helpers */

/* Interfaces */

/* API */
size64 ht_index_for(size64 idx, size64 shift) {
  return idx >> shift & HT_MASK;
}

void ht_copy(void* dst, void* src, size64 cnt) {
  assert(cnt <= HT_MAXC);

  memcpy(dst, src, cnt*sizeof(void*));
}

/* Initialization */
