#ifndef rl_tpl_decl_hashmap_h
#define rl_tpl_decl_hashmap_h

#include "util/hmap.h"
#include "tpl/type.h"

/* API signatures */
#define HASHMAP( HM, K, V )						\
  typedef hmap_t TYPE(HM);						\
  hmap_t  *make_##HM( size_t n_keys, void **ini );			\
  void     init_##HM( hmap_t *hmap );					\
  void     init_##HM##_table( void **table, size_t cap );		\
  size_t   resize_##HM( hmap_t *hmap, size_t new_n_keys );		\
  void     rehash_##HM( hmap_t *hmap, void **table, size_t new_cap );	\
  void     free_##HM( hmap_t *hmap );					\
  void     clear_##HM( hmap_t *hmap );					\
  void   **HM##_locate( hmap_t *hmap, K key );				\
  V        HM##_get( hmap_t *hmap, K key );				\
  V        HM##_set( hmap_t *hmap, K key, V val );			\
  V        HM##_put( hmap_t *hmap, K key, V val );			\
  bool     HM##_add( hmap_t *hmap, K key, V val );			\
  bool     HM##_has( hmap_t *hmap, K key );				\
  bool     HM##_remove( hmap_t *hmap, K key )

#endif
