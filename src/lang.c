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

int rl_order(Value x, Value y) {
  int output;
  if (x == y) {
    output = 0;
  } else {
    
  }

  return output;
}

hash_t rl_hash(Value x) {
  (void)x;

  hash_t output = 0;

  return output;
}
