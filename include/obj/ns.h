#ifndef rascal_ns_h
#define rascal_ns_h

#include "obj.h"

DECL_OBJ(ns);

struct ns_t
{
  OBJ_HEAD;

  obj_t  *next;

  obj_t  *module;
  obj_t  *local;
  obj_t  *captured;
  obj_t  *global;
  obj_t  *macro;
};

// globals --------------------------------------------------------------------
extern obj_t *ToplevelNS;

#endif
