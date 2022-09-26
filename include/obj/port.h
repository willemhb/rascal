#ifndef rascal_port_h
#define rascal_port_h

#include <stdio.h>

#include "obj.h"
#include "read.h"


typedef struct port_t port_t;

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

// initialization
void port_init( void );

// convenience
#define is_port(val) (is_obj_type(val, port_type))
#define as_port(val) ((port_t*)as_ptr(val))

#endif
