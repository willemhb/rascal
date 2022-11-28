#ifndef rascal_tpl_htable_h
#define rascal_tpl_htable_h

#include "util/tpl/impl/type.h"

/* API signatures */

#define HASHMAP( HM, K, V )					\
  long  *HM##_locate( hashmap_t *h, K key );			\
  V      HM##_get( hashmap_t *h, K key );			\
  V      HM##_set( hashmap_t *h, K key, V val );		\
  bool   HM##_put( hashmap_t *h, K key, V val );		\
  V      HM##_pop( hashmap_t *h, K key );			\
  bool   HM##_has( hashmap_t *h, K key );			\
  bool   HM##_remove( hashmap_t *h, K key );			\
  void **HM##_bp( hashmap_t *h, K key )

#define HASHSET( HS, K )				\
  long *HS##_locate( hashset_t *h, K key );		\
  bool  HS##_has( hashset_t *h, K key );		\
  bool  HS##_add( hashset_t *h, K key );		\
  bool  HS##_remove( hashset_t *h, K key )

#endif
