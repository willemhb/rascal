#ifndef rascal_common_h
#define rascal_common_h

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>

// utility typedefs
typedef uint8_t  byte_t;

typedef int32_t  order_t;
typedef uint32_t arity_t;
typedef uint32_t flags_t;
typedef uint64_t hash_t;
typedef uint64_t idno_t;

// general convenience 
#define unlikely(test) __builtin_expect(0, (test))

#define unreachable __builtin_unreachable

#define dispatch(test, x1, x2)			\
  (__builtin_choose_expr((test), (x1), (x2)))

#define types_compat(t1, t2)			\
  (__builtin_types_compatible_p(t1, t2))

static inline bool flag_p( flags_t fl, uint32_t mask )
{
  return !!(fl & mask);
}

static inline bool mflag_p( flags_t fl, uint32_t mask, uint32_t val )
{
  return (fl&mask) == val;
}

#endif
