#ifndef rl_tpl_decl_buffer_h
#define rl_tpl_decl_buffer_h

#include "common.h"
#include "tpl/type.h"

/* encoded arraylist, ie, ensures that elements includes a null terminator. */

#define BUFFER( B, X )					\
  typedef struct					\
  {							\
    size_t  len;					\
    size_t  cap;					\
    X      *elements;					\
  } TYPE(B);						\
  							\
  TYPE(B) *make_##B( size_t len );			\
  void resize_##B( TYPE(B) *B, size_t new_len );        \
  void reset_##B( TYPE(B) *B );			\
  void free_##B( TYPE(B) *buffer );		\
  void B##_push( TYPE(B) *B, X elt ); \
  X B##_pop( TYPE(B) *B ); \
  void B##_pushn( TYPE(B) *B, size_t n, ... );	\
  X B##_popn( TYPE(B) *B, size_t n ); \
  size_t writeto_##B( TYPE(B) *dst, X *src, size_t n ); \
  size_t appendto_##B( TYPE(B) *dst, X *src, size_t n );	\
  size_t readfrom_##B( X *dst, TYPE(B) *src, size_t n ); \
  X* B##_at( TYPE(B) *B, long i );		 \
  X B##_ref( TYPE(B) *B, long i ); \
  X B##_set( TYPE(B) *B, long i, X x ); \
  X B##_swap( TYPE(B) *B, long i, X x )

#endif
