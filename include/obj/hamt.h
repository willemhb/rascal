#ifndef rascal_hamt_h
#define rascal_hamt_h

#include "obj/type.h"

// amt
// funciton pointer types
typedef size_t     (*atlength_t)( amt_t *amt );
typedef size_t     (*atsize_t)( amt_t *amt );
typedef void       (*atresize_t)( amt_t **amt, size_t newl );

/* mutators work similarly to their counterparts in  array.h,
   but with an extra level of indirection (in case a new node needs to be created).
   They also take an extra argument indicating whether the array can be updated in-place */

typedef rl_value_t (*atref_t)(amt_t **amt, size_t n );
typedef rl_value_t (*atset_t)(amt_t **amt, size_t n, rl_value_t *buf, rl_value_t val, bool mut );
typedef size_t     (*atput_t)(amt_t **amt, size_t n, bool mut );
typedef rl_value_t (*atpop_t)(amt_t **amt, size_t n, bool mut );

struct amt_t
{
  BOX

  size_t  arity;
  size_t  bitmap;

  ushort  length;
  ushort  capacity;
  ushort  height;
  ushort  leaf;

  amt_t  *cache;

  byte    data[];
};

struct at_impl_t
{
  type_t    *eltype;
  uint       elsize;
  uint       unboxed;

  atsize_t   atsize;
  atlength_t atlength;
  atresize_t atresize;
  atref_t    atref;
  atset_t    atset;
  atput_t    atput;
  atpop_t    atpop;
};

// hamt
// function pointer types
typedef int        (*htcompare_t)( rl_value_t x, rl_value_t y );
typedef size_t     (*htlength_t)( hamt_t *hamt );
typedef size_t     (*htsize_t)( hamt_t *hamt );
typedef void       (*htresize_t)( hamt_t **hamt, size_t newl );

/* mutators work similarly to their counterparts in table.h,
   but with an extra level of indirection (in case a new node needs to be created).
   They also take an extra argument indicating whether the array can be updated in-place */

typedef bool       (*htref_t)(hamt_t **hamt, rl_value_t key, rl_value_t *buf );
typedef bool       (*htset_t)(hamt_t **hamt, rl_value_t key, rl_value_t *buf, rl_value_t val, bool mut );
typedef bool       (*htput_t)(hamt_t **hamt, rl_value_t key, rl_value_t *buf, bool mut );
typedef bool       (*htpop_t)(hamt_t **hamt, rl_value_t key, rl_value_t *buf, bool mut );

struct hamt_t
{
  BOX
  size_t  arity;
  size_t  bitmap;

  ushort  length;
  ushort  size;
  ushort  height;
  ushort  leaf;

  hamt_t     *cache;
  function_t *compare;

  byte    data[];
};

struct ht_impl_t
{
  size_t       entry_size;

  type_t      *keytype;
  type_t      *valtype;  // if none, table is not mapped

  // table methods
  htcompare_t  htcompare;
  htlength_t   htlength;
  htsize_t     htsize;
  htresize_t   htresize;
  htref_t      htref;
  htset_t      htset;
  htput_t      htput;
  htpop_t      htpop;
};

// 

#endif
