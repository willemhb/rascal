#ifndef rascal_common_h
#define rascal_common_h
#include <uchar.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>

// utility & style typedefs
typedef char     char8_t;
typedef uint8_t  byte;

typedef uint8_t  flags8_t;
typedef uint16_t flags16_t;
typedef uint16_t arity16_t;

typedef int32_t  ord32_t;
typedef uint32_t arity32_t;
typedef uint32_t flags32_t;
typedef uint32_t hash32_t;
typedef uint32_t idno32_t;

typedef ord32_t   ord_t;
typedef arity32_t arity_t;
typedef flags32_t flags_t;

typedef int64_t  ord64_t;
typedef uint64_t arity64_t;
typedef uint64_t hash64_t;
typedef uint64_t idno64_t;

typedef hash64_t hash_t;
typedef idno64_t idno_t;

// general convenience 
#define unlikely(test) __builtin_expect(0, (test))

#define unreachable __builtin_unreachable

#define dispatch(test, x1, x2)			\
  (__builtin_choose_expr((test), (x1), (x2)))

#define types_compat(t1, t2)			\
  (__builtin_types_compatible_p(t1, t2))

static inline bool flag_p( flags32_t fl, flags32_t mask )
{
  return !!(fl & mask);
}

static inline bool mflag_p( flags32_t fl, flags32_t mask, flags32_t val )
{
  return (fl&mask) == val;
}

#endif
