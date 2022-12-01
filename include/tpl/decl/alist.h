#ifndef rl_util_decl_alist_h
#define rl_util_decl_alist_h

#include "rascal.h"

#include "util/alist.h"
#include "tpl/type.h"

#define ALIST( AL, X )						\
  typedef alist_t TYPE(AL);					\
  alist_t *make_##AL( size_t len, X *ini );			\
  void     resize_##AL( alist_t *alist, size_t new_len );	\
  void     reset_##AL( alist_t *alist );			\
  void     free_##AL( alist_t *alist );				\
  size_t   AL##_push( alist_t *alist, X x );			\
  X        AL##_pop( alist_t *alist );				\
  size_t   AL##_pushn( alist_t *alist, size_t n, ... );		\
  X        AL##_popn( alist_t *alist, size_t n );		\
  size_t   appendto_##AL( alist_t *alist, X *src, size_t n );	\
  size_t   writeto_##AL( alist_t *alist, X *src, size_t n );	\
  size_t   readfrom_##AL( alist_t *alist, X *dst, size_t n );	\
  X       *AL##_at( alist_t *alist, long i );			\
  X        AL##_ref( alist_t *alist, long i );			\
  X        AL##_set( alist_t *alist, long i, X x );		\
  X        AL##_swap( alist_t *alist, long i, X x )

#endif
