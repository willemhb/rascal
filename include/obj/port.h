#ifndef rascal_port_h
#define rascal_port_h

#include <stdio.h>

#include "obj.h"
#include "read.h"


DECL_OBJ(port);
DECL_OBJ_API(port);

struct port_t
{
  OBJ_HEAD;

  val_t    value;
  obj_t   *buffer;

  FILE    *stream;

  tok_t    token;
  flags_t  flags;
};

// globals --------------------------------------------------------------------
extern obj_t *Ins, *Outs, *Errs;

#endif
