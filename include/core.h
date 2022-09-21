#ifndef rascal_core_h
#define rascal_core_h

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef int32_t  arity_t;
typedef uint32_t flags_t;
typedef uint64_t hash_t;
typedef uint64_t idno_t;

// static utils ---------------------------------------------------------------
static inline bool is_flag( flags_t fl, uint32_t mask )
{
  return !!(fl & mask);
}

#endif
