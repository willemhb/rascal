#ifndef rl_util_decl_buffer_h
#define rl_util_decl_buffer_h

#include "common.h"
#include "util/tpl/impl/type.h"

/* encoded arraylist, ie, ensures that elements includes a null terminator. */

#define BUFFER( X )					\
  struct						\
  {							\
    size_t  len;					\
    size_t  cap;					\
    X      *elements;					\
  }

#define BUFSIZE( X ) sizeof(BUFFER(X))

#define MAKE_BUFFER( B, X )			\
  TYPE(B) *make_##B( size_t len )

#define RESIZE_BUFFER( B, X )			\
  void resize_##B( TYPE(B) *B, size_t new_len )

#define FREE_BUFFER( B, X )			\
  void free_##B( TYPE(B) *buffer )

#define BUFFER_PUSH( B, X )			\
  void B##_push( TYPE(B) *B, X elt )

#define BUFFER_POP( B, X )			\
  X B##_pop( TYPE(B) *B )

#define BUFFER_PUSHN( B, X )			\
  void B##_pushn( TYPE(B) *B, size_t n, ... )

#define BUFFER_POPN( B, X )			\
  X B##_popn( TYPE(B) *B, size_t n )

#define WRITETO_BUFFER( B, X )				\
  size_t writeto_##B( TYPE(B) *dst, X *src, size_t n )

#define READFROM_BUFFER( B, X )				\
  size_t readfrom_##B( X *dst, TYPE(B) *src, size_t n )

#define BUFFER_REF( B, X )			\
  X B##_ref( TYPE(B) *B, long i )

#define BUFFER_SET( B, X )			\
  X B##_set( TYPE(B) *B, long i, X x )

#define BUFFER_SWAP( B, X )			\
  X B##_swap( TYPE(B) *B, long i, X x )

#endif
