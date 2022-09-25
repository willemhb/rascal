#ifndef rascal_core_h
#define rascal_core_h

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>

// utility typedefs -----------------------------------------------------------
typedef uint8_t  byte_t;

typedef uint32_t arity_t;
typedef uint32_t flags_t;
typedef uint64_t hash_t;
typedef uint64_t idno_t;

typedef double     real_t;
typedef intptr_t   int_t;
typedef char       char_t;
typedef bool       bool_t;
typedef void      *ptr_t;

// convenience ----------------------------------------------------------------
#define unlikely(test) __builtin_expect(0, (test))

#define dispatch(test, x1, x2)			\
  (__builtin_choose_expr((test), (x1), (x2)))

#define types_compat(t1, t2)			\
  (__builtin_types_compatible_p(t1, t2))

static inline bool flag_p( flags_t fl, uint32_t mask )
{
  return !!(fl & mask);
}

#endif
