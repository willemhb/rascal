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
/* API */
#define is_vnode(x) has_type(x, T_VNODE)
#define as_vnode(x) ((VNode*)as_obj(x))

VNode* mk_vnode(void* d, size32 c, size32 s);
VNode* mk_vleaf(Val* vs);
VNode* last_child(VNode* n);
VNode* last_leaf(VNode* n);
VNode* vnode_set(VNode* n, size64 i, Val x);
VNode* push_leaf(VNode* n, Val* vs);
VNode* pop_leaf(VNode* n, VNode** lf);
bool   vn_egal_traverse(VNode* x, VNode* y);
size64 vn_pr_traverse(State* vm, Port* p, VNode* n);

/* Initialization */

#endif
