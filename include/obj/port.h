#ifndef rascal_port_h
#define rascal_port_h

#include "obj.h"
#include "read.h"


// C types --------------------------------------------------------------------
typedef struct port_t
{
  OBJ_HEAD;

  val_t    stream;
  val_t    value;
  obj_t   *buffer;

  tok_t    token;
  flags_t  flags;
} port_t;

#endif
