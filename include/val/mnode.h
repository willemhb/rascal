#ifndef rl_val_mnode_h
#define rl_val_mnode_h

#include "val/object.h"

/* Internal node type for Maps (see val/map.h). */
/* C types */


/* Globals */

/* API */
#define is_mnode(x) has_type(x, T_MNODE)
#define as_mnode(x) ((MNode*)as_obj(x))

MNode* new_mnode(size64 shft);

/* Initialization */

#endif
