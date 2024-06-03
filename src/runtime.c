#include <string.h>

#include "runtime.h"
#include "util.h"

/* Globals. */
#define INITIAL_HEAP_SIZE (1 << 19) // 2^16 * sizeof(Value)
#define MAXIMUM_HEAP_SIZE MAX_ARITY
#define STACK_SIZE (1 << 16)

/* Buffer APIs. */
#define BUFFER_API(T, _T, t, e)                                         \
  rl_status_t init_##t##_buffer(T##Buffer* b, _T* f, size_t fm) {       \
    b->base = f;                                                        \
    b->fast = f;                                                        \
    b->next = 0;                                                        \
    b->max  = fm;                                                       \
    b->fmax = fm;                                                       \
                                                                        \
    return OKAY;                                                        \
  }                                                                     \
                                                                        \
  rl_status_t free_##t##_buffer(T##Buffer* b) {                         \
    rl_status_t out = OKAY;                                             \
    if ( b->base != b->fast ) {                                         \
      out = deallocate(b->base, 0, false);                              \
      if ( out != OKAY )                                                \
        goto end;                                                       \
    }                                                                   \
                                                                        \
    out = init_##t##_buffer(b, b->fast, b->fmax);                       \
  end:                                                                  \
    return out;                                                         \
  }                                                                     \
                                                                        \
  rl_status_t resize_##t##_buffer(T##Buffer* b, size_t *n) {            \
    rl_status_t out = OKAY;                                             \
    size_t ns = *n;                                                     \
    size_t np = ns + e;                                                 \
    bool   os   = b->max > b->fmax;                                     \
    if ( unlikely(np > b->max) ) {                                      \
      size_t nm = ceil2(np);                                            \
      size_t om = b->max;                                               \
      if ( os ) {                                                       \
        void* buf = &b->base;                                           \
        out = reallocate(&buf, om*sizeof(_T), nm*sizeof(_T), false);    \
        if ( out != OKAY )                                              \
          goto end;                                                     \
        b->base = buf;                                                  \
      } else {                                                          \
        void* buf   = &b->base;                                         \
        size_t next = b->next;                                          \
        out = allocate(&buf, nm*sizeof(_T), false);                     \
        if ( out != OKAY )                                              \
          goto end;                                                     \
        memcpy(buf, b->fast, next*sizeof(_T));                          \
        memset(b->fast, 0, next*sizeof(_T));                            \
        b->base = buf;                                                  \
      }                                                                 \
      b->max = nm;                                                      \
    } else if ( unlikely(os && np < (b->max >> 1)) ) {                  \
      size_t nm = ceil2(np);                                            \
    }                                                                   \
                                                                        \
      b->next = ns;                                                     \
      *n      = b->max;                                                 \
                                                                        \
  end:                                                                  \
    return out;                                                         \
  }

BUFFER_API(Value, Value, value, false);
BUFFER_API(Text, char, text, true);
BUFFER_API(Object, Object*, object, false);
BUFFER_API(RFrame, RFrame, rframe, false);
BUFFER_API(CFrame, CFrame, cframe, false);
BUFFER_API(IFrame, IFrame, iframe, false);

#undef BUFFER_API
