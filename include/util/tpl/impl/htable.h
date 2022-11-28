#ifndef rl_util_tpl_impl_htable_h
#define rl_util_tpl_impl_htable_h

#include "util/tpl/impl/type.h"
#include "util/tpl/undef/htable.h"

#define HASHMAP( HM, K, V, hashfn, cmpfn, notfound )		\
  long *HM##_locate( hashmap_t *hm, K key )			\
  {								\
    ulong h  = hashfn(key);					\
    ulong i  = h & (hm->cap-1), n;				\
    long *o  = hm->order;					\
    void **t = hm->table;					\
    while ( (n=o[i*2]) > -1 )					\
      {								\
        if ( (ulong)o[i*2+1] == h && cmpfn(*(K*)(t+n*2), key) )	\
	  break;						\
	i = (i+1) & (hm->cap-1);				\
      }								\
    return o+i*2;						\
  }								\
								\
  V HM##_get( hashmap_t *hm, K key )				\
  {								\
    long *o = HM##_locate(hm, key);				\
    if ( *o == -1 )						\
      return notfound;						\
    return *(V*)(vm->table+(*o)*2+1);				\
  }								\
								\
  V HM##_set( hashmap_t *hm, K key, V val )			\
  {								\
    long *o = HM##_locate(hm, key);				\
    if ( *o == -1 )						\
      return notfound;						\
    *(V*)(vm->table+(*o)*2+1) = val;				\
    return val;							\
  }								\
								\
  bool HM##_put( hashmap_t *hm, K key, V val )			\
  {								\
    long *o = HM##_locate(hm, key);				\
    if ( *o == -1 )						\
      {								\
	o[0] = hm->len;						\
	o[1] = hashfn(key);					\
	*(K*)(hm->table+hm->len*2)   = key;			\
	*(V*)(hm->table+hm->len*2+1) = val;			\
	resize_hashmap(hm, hm->len+1);				\
	return true;						\
      }								\
    else							\
      return false;						\
  }								\
  								\
  V HM##_pop( hashmap_t *hm, K key )				\
  {								\
    long *o = HM##_locate(hm, key);				\
    if ( *o == -1 )						\
      return notfound;						\
    long n = *o;						\
    V out  = *(V*)(hm->table+n*2+1);				\
  }								\
  								\
  bool HM##_has( hashmap_t *hm, K key )				\
  {								\
    long *o = HM##_locate(hm, key);				\
    return *o != -1;						\
  }								\
								\

#endif
