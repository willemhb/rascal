#ifndef rl_type_h
#define rl_type_h

#include "rascal.h"

/* C types */
struct type_t {
  /* general metadata */
  char *name;

  /* metaobject methods */
  isa_fn_t isa;

  /* common methods */
  prin_fn_t prin;

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

/* API */
size_t base_size(type_t type);
size_t alloc_size(type_t type, size_t n);

#endif
