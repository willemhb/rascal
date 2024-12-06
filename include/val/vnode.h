#ifndef rl_val_vnode_h
#define rl_val_vnode_h

#include "val/object.h"

/* C types */
struct VNode {
  HEADER;

  bool full;         // indicates whether the node is full
  size16 cnt, shft;

  union {
    VNode** cn;
    Val*    vs;
  };
};

/* Globals */

/* External APIs */
#define is_vnode(x) has_type(x, T_VNODE)
#define as_vnode(x) ((VNode*)as_obj(x))

// initialization

#endif
