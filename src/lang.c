#include "lang.h"


/* top-level APIs. */
bool rl_egal(Value x, Value y) {
  bool output;
  if (x == y) {
    output = true;
  } else {
    
  }

  return output;
}

int rl_order(Value x, Value y);
hash_t rl_hash(Value x);
