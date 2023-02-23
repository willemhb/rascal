#include "util/collections.h"
#include "util/number.h"

// size padding ---------------------------------------------------------------
usize pad_stack_size(usize count, usize cap) {
  cap = MAX(cap, MinSCap);
  cap = ceil2(cap);

  if (count >= cap || (count <= (cap >> 1) && cap > MinSCap))
    cap = ceil2(count+1);

  return cap;
}

usize pad_table_size(usize count, usize cap) {
  cap = MAX(cap, MinTCap);
  cap = ceil2(cap);

  if (count >= cap * LoadF || (count <= (cap >> 1) * LoadF && cap > MinTCap))
    cap = ceil2(count+1);

  return cap;
}

usize pad_alist_size(usize oldct, usize newct, usize oldcap) {
  if (oldcap == 0)
    oldcap = MinACap;

  if (oldcap > newct && (newct > (oldcap >> 1) || oldcap == MinACap))
    return oldcap;

  /* The over-allocation is mild, but is enough to give linear-time amortized 
   * behavior over a serial calls to push() in the presence of a poorly-performing
   * system realloc().
   *
   * Add padding to make the allocated size multiple of 4.
   * The growth pattern is:  0, 4, 8, 16, 24, 32, 40, 52, 64, 76, ...
   */
  
  usize newcap = (newct + (newct >> 3) + 6) & ~3ul;
    /* Do not overallocate if the new size is closer to overallocated size
     * than to the old size.
     */

    if (newct - oldct > newcap - newct)
      newcap = (newct + 3) & ~3ul;

    return newcap;
}
