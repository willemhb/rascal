#ifndef rl_tpl_impl_hashset_h
#define rl_tpl_impl_hashset_h

#include "util/collection.h"
#include "util/memory.h"

#include "tpl/type.h"

#define HASHSET( HS, M, padfn, hashfn, cmpfn, nomember )		\
  hset_t *make_##HS( size_t n_keys, void **ini )			\
  {									\
    hset_t *hset = make_hset(n_keys, padfn);				\
    /* initialize empty cells */					\
    init_##HS(hset);							\
    if ( ini )								\
      {									\
	for ( size_t i=0; i<n_keys; i++ )				\
	  {								\
	    M member = hset_member(ini, i, M);				\
	    HS##_add(hset, member);					\
	  }								\
      }									\
    return hset;							\
  }									\
  									\
  void init_##HS( hset_t *hset )					\
  {									\
    init_##HS##_table(hset->table, hset->cap)				\
  }									\
  									\
  void init_##HS##_table( void **table, size_t cap )			\
  {									\
    for ( size_t i=0; i<cap; i++ )					\
      hset_member(table, i, M) = nomember;				\
  }									\
  									\
  size_t resize_##HS( hset_t *hset, size_t new_n_keys )			\
  {									\
    size_t new_cap = padfn(new_n_keys, hset->len, hset->cap);		\
    if ( new_cap != hset->cap )						\
      {									\
	void **table = calloc_s(new_cap, sizeof(void*));		\
	rehash_##HS(hset, old_table, new_cap);				\
	free(hset->table);						\
	hset->table      = table;					\
	hset->cap        = new_cap;					\
      }									\
    hset->len = new_n_keys;						\
    return hset->len;							\
  }									\
									\
  void rehash_##HS( hset_t *hset, void **table, size_t new_cap )	\
  {									\
    init_##HS##_table(table, new_cap);					\
    									\
    for ( size_t i=0, n=0; i<hset->cap && n < hset->len; i++)		\
      {									\
	M member = hset_member(hset, i, M);				\
	if ( member == nomember )					\
	  continue;							\
	ulong hash = hashfn(member);					\
	ulong idx  = hash & (new_cap-1);				\
	while ( *(M*)(table+idx) != nomember )				\
	  idx = idx+1 & (new_cap-1);					\
	*(M*)(table+idx) = member;					\
	n++;								\
      }									\
  }									\
  									\
  void free_##HS( hset_t *hset ) { free_hset(hset); }			\
  									\
  void clear_##HS( hset_t *hset )					\
  {									\
    clear_hset(hset);							\
    init_##HS##_table(hset);						\
  }									\
									\
  void **HS##_locate( hset_t *hset, M member )				\
  {									\
    ulong hash  = hashfn(member);					\
    ulong index = hash & (hset->cap-1);					\
    M hmember;								\
    while ( (hmember=hset_member(hset, index, M)) != nomember )		\
      {									\
	if ( cmpfn(hmember, member) )					\
	  break;							\
	index = (index+1) & (hset->cap-1);				\
      }									\
    return hset_cell(hset, index);					\
  }									\
  									\
  bool HS##_add( hset_t *hset, M member )				\
  {									\
    void **cell = HS##_locate(hset, member);				\
    if (*(M*)cell == nomember )						\
      {									\
	*(M*)cell     = member;						\
	resize_##HS(hset, hset->len+1);					\
	return true;							\
      }									\
    return false;							\
  }									\
  									\
  bool HS##_has( hset_t *hset, M member )				\
  {									\
    void **cell = HS##_locate(hset, member);				\
    return *(M*)cell != nomember;					\
  }									\
  									\
  bool HS##_remove( hset_t *hset, M member )				\
  {									\
    void **cell = HS##_locate(hset, member);				\
    if (*(M*)cell == nomember )						\
      return false;							\
    *(M*)cell     = nomember;						\
    resize_##HS(hset, hset->len-1);					\
    return true;							\
  }

#endif
