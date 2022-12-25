#ifndef rl_tpl_decl_hashset_h
#define rl_tpl_decl_hashset_h

#include "util/hset.h"

#define HASHSET( HS, M )                                                \
  hset_t  *make_##HS( size_t n_keys, void **ini );                      \
  void     init_##HS##_table( hset_t *hset );                           \
  size_t   resize_##HS( hset_t *hset, size_t new_n_keys );              \
  void     rehash_##HS( hset_t *hset, void ** table, size_t new_cap );	\
  void     free_##HS( hset_t *hset );                                   \
  void     clear_##HS( hset_t *hset );                                  \
  void   **HS##_locate( hset_t *hset, M member );                       \
  bool     HS##_has( hset_t *hset, M member );                          \
  bool     HS##_add( hset_t *hset, M member );                          \
  bool     HS##_remove( hset_t *hset, M member )


#endif
