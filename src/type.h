#ifndef rl_type_h
#define rl_type_h

#include "rascal.h"

/* C types */
struct type_t {
  /* general metadata */
  char *name;
  ulong idno;

  /* metaobject methods */
  isa_fn_t isa;

  /* common methods */
  prin_fn_t    prin;
  hash_fn_t    hash;
  compare_fn_t compare;
  equal_fn_t   equal;

  /* object methods */
  create_fn_t  create;
  resize_fn_t  resize;
  init_fn_t    init;
  runtime_fn_t free;
  pad_fn_t     pad;

  /* object layout */
  size_t head_size;
  size_t base_offset;
  size_t body_size;
  size_t el_size;
};

/* globals */
// builtin type idnos
enum {
  // fucked up types
  none_type_idno,
  any_type_idno,
  nul_type_idno,

  // core user types
  sym_type_idno,
  func_type_idno,
  cons_type_idno,
  vec_type_idno,
  str_type_idno,
  dict_type_idno,
  set_type_idno,
  bool_type_idno,
  small_type_idno,
  real_type_idno,

  // internal types
  method_type_idno,
  module_type_idno,
  native_type_idno,
  code_type_idno,

  // union types
  list_type_idno,
  arr_type_idno,
  table_type_idno,
};

/* API */
size_t base_size(type_t type);
size_t alloc_size(type_t type, size_t n);

#endif
