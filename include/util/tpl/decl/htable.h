#ifndef rascal_tpl_htable_h
#define rascal_tpl_htable_h

#include "util/tpl/impl/type.h"

/* API signatures */

#define HTABLE(HT, K, V )					\
  void  *HT##_get( htable_t *h, K key );			\
  bool   HT##_put( htable_t *h, K key, V val );			\
  bool   HT##_adjoin( htable_t *h, K key, V val );		\
  bool   HT##_has( htable_t *h, K key, V val );			\
  bool   HT##_remove( htable_t *h, K key );			\
  bool   HT##_pop( htable_t *h, K key, V *buf );		\
  void **HT##_bp( htable_t *h, K key )

#endif
