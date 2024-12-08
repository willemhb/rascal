#include "val/vnode.h"
#include "val/seq.h"

#include "lang/compare.h"

#include "vm/heap.h"

#include "util/amt.h"

/* C types */

/* Forward declarations */
// Helpers
static VNode* new_vnode(void);
static void   init_vnode(VNode* n);

// Interfaces
void trace_vnode(State* vm, void* x);
void free_vnode(State* vm, void* x);
void clone_vnode(State* vm, void* x);
void seal_vnode(State* vm, void* x, bool d);
void vnode_sinit(Seq* s);
Val  vnode_sfirst(Seq* s);
void vnode_srest(Seq* s);

// External
/* Globals */
VTable VNodeVt = {
  .code    =T_VNODE,
  .name    ="VNode",
  .obsize  =sizeof(VNode),
  .is_iseq =true,
  .tracefn =trace_vnode,
  .freefn  =free_vnode,
  .clonefn =clone_vnode,
  .sealfn  =seal_vnode,
  .sinitfn =vnode_sinit,
  .sfirstfn=vnode_sfirst,
  .srestfn =vnode_srest,
};

/* Helpers */
static VNode* new_vnode(void) {
  // create and initialize vnode
  VNode* n = new_obj(&Vm, T_VEC, 0); init_vnode(n);

  // return new vnode
  return n;
}

static void init_vnode(VNode* n) {
  n->cnt  = 0;
  n->shft = 0;
  n->full = false;
  n->vs   = rl_alloc(NULL, HT_MAXS);
}

/* Interfaces */
void trace_vnode(State* vm, void* x) {
  VNode* n = x;

  if ( n->shft == 0 )
    mark_vals(vm, n->cnt, n->vs);

  else
    mark_objs(vm, n->cnt, n->cn);
}

void free_vnode(State* vm, void* x) {
  (void)vm;

  VNode* n = x;

  rl_dealloc(NULL, n->vs, 0);
}

void clone_vnode(State* vm, void* x) {
  VNode* n = x;

  n->cn = rl_dup(vm, n->cn, HT_MAXS);
}

void seal_vnode(State* vm, void* x, bool d) {
  VNode* n = x;

  if ( d && n->shft > 0 )
    for ( size64 i=0; i < n->cnt; i++ )
      seal_obj(vm, n->cn[i], d);
}

// sequence interface
void vnode_sinit(Seq* s) {
  VNode* n = s->src;
  
  if ( n->shft )
    s->cseq = mk_seq(n->cn[0], s->sealed);
}

Val vnode_sfirst(Seq* s) {
  if ( s->fst == NOTHING ) {
    if ( s->cseq )
      s->fst = vnode_sfirst(s->cseq);

    else { // should be a leaf node
      VNode* n = s->src;
      s->fst   = n->vs[s->off];
    }
  }

  return s->fst;
}

/* API */
VNode* mk_vnode(void* d, size32 c, size32 s) {
  VNode* n = new_vnode();
  n->cnt   = c;
  n->shft  = s;
  n->full  = false;

  if ( d ) {
    ht_copy(n->cn, d, c);
    n->full = c == HT_MAXC && (s == 0 || ((VNode**)d)[c-1]->full);
  }

  return n;
}

VNode* mk_vleaf(Val* vs) {
  // create and initialize new leaf
  VNode* n = new_vnode();
  n->cnt   = HT_MAXC;
  n->shft  = 0;
  n->full  = true;

  ht_copy(n->vs, vs, HT_MAXC);

  // return the leaf
  return n;
}

VNode* last_child(VNode* n) {
  assert(n->cnt > 0);
  assert(n->shft > 0);

  return n->cn[n->cnt-1];
}

VNode* last_leaf(VNode* n) {
  while ( n->shft > 0 )
    n = last_child(n);

  return n;
}


VNode* vnode_set(VNode* n, size64 i, Val x) {
  if ( n->sealed ) { // unseal, save, set, and seal
    n = unseal_obj(&Vm, n); preserve(&Vm, 1, tag(n));
    n = vnode_set(n, i, x);
    n = seal_obj(&Vm, n, false);
  } else if ( n->shft == 0 ) {        // leaf, insert at appropriate index
    n->vs[i & HT_MASK] = x;
  } else {                            // insert in appropriate child
    size64 idx = ht_index_for(i, n->shft);
    VNode* c   = n->cn[idx];
    n->cn[idx] = vnode_set(c, i, x);
  }

  return n;
}

VNode* push_leaf(VNode* n, Val* vs) {
  if ( n->sealed ) {
    n = unseal_obj(&Vm, n); preserve(&Vm, 1, tag(n));
    n = push_leaf(n, vs);
    n = seal_obj(&Vm, n, false);

  } else if ( n->shft == HT_SHFT ) {
    assert(!n->full);   // should be checked before insertion at child

    // add leaf and update full
    n->cn[n->cnt++] = mk_vleaf(vs);

  } else if ( n->cnt == 0 || n->cn[n->cnt-1]->full ) {
    // create new empty level and add there
    n->cn[n->cnt++] = mk_vnode(NULL, 0, n->shft-HT_SHFT);
    push_leaf(n->cn[n->cnt-1], vs);
  } else
    n->cn[n->cnt-1] = push_leaf(n->cn[n->cnt-1], vs);

  // check whether the current node is now full
  n->full = n->cnt == HT_MAXC && n->cn[n->cnt-1]->full;

  return n;
}

VNode* pop_leaf(VNode* n, VNode** lf) {
  /**
   * the control is a little goofy here because we want to avoid making
   * unnecessary copies that won't get used. So we check if a node will be used
   * after popping (possibly calling pop_leaf recursively) before deciding whether
   * the current node needs to be copied preserved. This leads to a lot of duplication
   * of the "unseal, modify, seal" sequence that we normally try to do once up front,
   * but it's actually a lot simpler that way.
   **/

  if ( n->shft == HT_SHFT ) {
    *lf = last_child(n);

    if ( n->cnt == 1 )
      n = NULL;

    else if ( n->sealed ) {
      // create a copy to modify
      n = unseal_obj(&Vm, n);
      n->cnt--;
      n = seal_obj(&Vm, n, false);
    } else {
      n->cnt--;
    }
  } else {
    VNode* cn = pop_leaf(last_child(n), lf);

    if ( cn == NULL ) {
      if ( n->cnt == 1 )
        n = NULL;
      else if ( n->sealed ) {
        n = unseal_obj(&Vm, n);
        n->cnt--;
        n = seal_obj(&Vm, n, false);
      } else {
        n->cnt--;
      }
    } else if ( n->sealed ) {
      preserve(&Vm, 1, tag(cn));
      n = unseal_obj(&Vm, n);
      n->cn[n->cnt-1] = cn;
      n = seal_obj(&Vm, n, false);
    } else {
      n->cn[n->cnt-1] = cn;
    }
  }

  return n;
}

bool vn_egal_traverse(VNode* x, VNode* y) {
  /* only called when two vectors are known to have the same length, so the vnodes
     are assumed to have the same structure. */
  bool r = true;

  if ( x->shft > 0 ) {
    for ( size64 i=0; r && i < HT_MAXC; i++ )
      r = rl_egal(x->vs[i], y->vs[i]);
  } else {
    for ( size64 i=0; r && i < x->cnt; i++ )
      r = vn_egal_traverse(x->cn[i], y->cn[i]);
  }

  return r;
}

size64 pr_vnode_vals(State* vm, Port* p, VNode* n) {
  size64 o = 0;

  if ( n ) {
    if ( n->shft == 0 ) {
      for ( size64 i=0; i < TL_CNT; i++ ) {
        o += rl_pr(p, n->vs[i]);
        o += rl_putc(p, ' ');
      }
    } else {
      for ( size64 i=0; i < n->cnt; i++ ) {
        o += pr_vnode_vals(vm, p, n->cn[i]);
      }
    }
  }

  return o;
}

/* Initialization */
