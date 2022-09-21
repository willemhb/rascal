#ifndef rascal_atom_h
#define rascal_atom_h

#include "obj.h"
#include "template/table.h"

typedef struct atom_t
{
  OBJ_HEAD;
  obj_t     *name;
  hash_t     hash;
} atom_t;



#endif
