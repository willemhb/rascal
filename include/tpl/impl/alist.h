#ifndef rl_tpl_impl_alist_h
#define rl_tpl_impl_alist_h

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>

#include "tpl/decl/generic.h"

#include "util/alist.h"
#include "util/number.h"
#include "util/collection.h"

#undef ALIST

#define ALIST( AL, X, V, padfn )                                \
  alist_t *make_##AL( size_t len, X *ini )                      \
  {                                                             \
    alist_t *alist = make_alist(len, padfn);                    \
    if ( ini ) {                                                \
        for ( size_t i=0; i<alist->cap; i++ )                   \
          alist_member(alist, i, X) = alist_member(ini, i, X);  \
    }                                                           \
    return alist;                                               \
  }                                                             \
                                                                \
  void resize_##AL( alist_t *alist, size_t new_len )			\
  {                                                             \
    resize_alist(alist, new_len, padfn);                        \
  }                                                             \
                                                                \
  void reset_##AL( alist_t *alist )                             \
  {                                                             \
    reset_alist(alist, padfn);                                  \
  }                                                             \
                                                                \
  void free_##AL( alist_t *alist )                              \
  {                                                             \
    free_alist(alist);                                          \
  }                                                             \
                                                                \
  size_t AL##_push( alist_t *alist, X x )                       \
  {                                                             \
    resize_##AL(alist, alist->len+1);                           \
    alist_member(alist, alist->len-1, X) = x;                   \
    return alist->len;                                          \
  }                                                             \
                                                                \
  X AL##_pop( alist_t *alist )                                  \
  {                                                             \
    X x = alist_member(alist, alist->len-1, X);                 \
    resize_##AL(alist, alist->len-1);                           \
    return x;                                                   \
  }                                                             \
  size_t AL##_pushn( alist_t *alist, size_t n, ... )		\
  {                                                             \
    size_t old_len = alist->len;                                \
    resize_##AL(alist, alist->len+n);                           \
    void **cell = alist->elements+old_len;                      \
    va_list va; va_start(va, n);                                \
    for ( size_t i=0; i<n; i++ )                                \
      *(X*)(cell++) = va_arg(va, V);                            \
    va_end(va);                                                 \
    return alist->len;                                          \
  }                                                             \
                                                                \
  X AL##_popn( alist_t *alist, size_t n )                       \
  {                                                             \
    X x = alist_member(alist, alist->len-1, X);                 \
    resize_##AL(alist, alist->len-n);                           \
    return x;                                                   \
  }                                                             \
                                                                \
  size_t AL##_trim(alist_t *alist, size_t n) {                  \
    resize_##AL(alist, n);                                      \
    return alist->len;                                          \
  }                                                             \
  size_t appendto_##AL( alist_t *alist, X *src, size_t n )	\
  {                                                             \
    size_t old_len = alist->len;                                \
    resize_##AL(alist, alist->len+n);                           \
    void **cell = alist->elements+old_len;                      \
    for ( size_t i=0; i<n; i++ )                                \
      *(X*)(cell++) = src[i];                                   \
    return alist->len;                                          \
  }                                                             \
  size_t writeto_##AL( alist_t *alist, X *src, size_t n )		\
  {                                                             \
    resize_##AL(alist, max(n, alist->len));                     \
    void **cell = alist->elements;                              \
    for ( size_t i=0; i<n; i++ )                                \
      *(X*)(cell++) = src[i];                                   \
    return n;                                                   \
  }                                                             \
                                                                \
  size_t readfrom_##AL( alist_t *alist, X *dst, size_t n )		\
  {                                                             \
    size_t wlen = min(n, alist->len);                           \
    void **cell = alist->elements;                              \
    for ( size_t i=0; i<n; i++ )                                \
      dst[i] = *(X*)(cell++);                                   \
    return wlen;                                                \
  }                                                             \
                                                                \
  X* AL##_at( alist_t *alist, long i )                          \
  {                                                             \
    if ( i < 0 )                                                \
      i += alist->len;                                          \
    assert(i >= 0 && (size_t)i < alist->len);                   \
    return (X*)(alist->elements+i);                             \
  }                                                             \
                                                                \
  X AL##_ref( alist_t *alist, long i )                          \
  {                                                             \
    return *AL##_at(alist, i);                                  \
  }                                                             \
                                                                \
  X AL##_set( alist_t *alist, long i, X x )                     \
  {                                                             \
    X* spc = AL##_at(alist, i);                                 \
    *spc = x;                                                   \
    return x;                                                   \
  }                                                             \
                                                                \
  X AL##_swap( alist_t *alist, long i, X x )                    \
  {                                                             \
    X* spc = AL##_at(alist, i);                                 \
    X  out = *spc;                                              \
    *spc   = x;                                                 \
    return out;                                                 \
  }

#endif
