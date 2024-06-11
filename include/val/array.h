#ifndef rl_array_h
#define rl_array_h

#include "val/object.h"

/* Types, APIs, and globals for mutable and immutable Rascal arrays. */
/* C types */
// vector root and node types
struct Vector {
  HEADER;

  // bit fields
  word_t transient : 1;
  word_t packed    : 1;

  // data fields
  size_t count;

  union {
    // common Vector
    struct {
      VecNode* root;
      Value    tail[0];
    };

    // packed Vector
    struct {
      Value    slots[0];
    };
  };
};

struct VecNode {
  HEADER;

  // bit fields
  word_t offset    : 6;
  word_t depth     : 4;
  word_t transient : 1;

  uint32_t count, max_count;

  // data fields
  union {
    Object** children;
    Value    slots[0];
  };
};

// dynamic array types
#define DYNAMIC_ARRAY(X)                        \
  word_t algo : 1;                              \
  X* data;                                      \
  X* _static;                                   \
  size_t count, max_count, max_static

struct MutVec {
  HEADER;

  // data fields
  DYNAMIC_ARRAY(Value);
};

struct Alist {
  HEADER;

  // data fields
  DYNAMIC_ARRAY(void*);
};

#undef DYNAMIC_ARRAY

/* Globals */
extern Type VectorType, VecNodeType, MutVecType, AlistType;

/* APIs */
/* Vector API */

/* Dynamic array APIs */
#define MUTABLE_ARRAY(T, t, X)                                          \
  rl_status_t init_##t(T* a, X* s, size_t ss);                          \
  rl_status_t free_##t(T* a);                                           \
  rl_status_t grow_##t(T* a, size_t n);                                 \
  rl_status_t shrink_##t(T* a, size_t n);                               \
  rl_status_t write_##t(T* a, X* s, size_t n);                          \
  rl_status_t t##_push(T* a, X x);                                      \
  rl_status_t t##_pushn(T* a, size_t n, ...);                           \
  rl_status_t t##_pushv(T* a, size_t n, va_list va);                    \
  rl_status_t t##_pop(T* a, X* r);                                      \
  rl_status_t t##_popn(T* a, X* r, bool e, size_t n)

MUTABLE_ARRAY(MutVec, mvec, Value);
MUTABLE_ARRAY(Alist, alist, void*);

#undef MUTABLE_ARRAY


#endif
