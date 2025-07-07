/* DESCRIPTION */
// headers --------------------------------------------------------------------

#include "data/list.h"

#include "lang/base.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
// internal helpers -----------------------------------------------------------
size_t tail_offset(List* list);
size_t index_for_node(ListNode* node, size_t offset);
size_t index_for_shift(size_t shift, size_t offset);
Expr*  array_for(List* list, size_t offset);

#define index_for(x, o)                         \
  generic((x),                                  \
          ListNode*:index_for_node,             \
          size_t:index_for_shift)(x, o)

// expression APIs ------------------------------------------------------------
void print_list(Port* ios, Expr x);
bool egal_lists(Expr x, Expr y);

// object APIs ----------------------------------------------------------------
void trace_list(void* ptr);

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------
size_t tail_offset(List* list) {
  if ( list->arity < HAMT_SIZE )
    return 0;

  return list->arity & HAMT_MASK;
}

size_t index_for_node(ListNode* node, size_t offset) {
  return (offset >> hamt_shift(node)) & HAMT_MASK;
}

size_t index_for_shift(size_t shift, size_t offset) {
  return (offset >> shift) & HAMT_MASK;
}

Expr* array_for(List* list, size_t offset) {
  assert(offset > 0 && offset < list->arity);

  if ( offset >= tail_offset(list) )
    return list->tail;

  else {
    Obj* node = (Obj*)list->root;

    for ( size_t shift=hamt_shift(node); shift > 0; shift -= HAMT_SHIFT ) {
      size_t index = index_for(shift, offset);
      
      node = ((ListNode*)node)->children[index];
    }

    return ((ListLeaf*)node)->slots;
  }
}

// initialization -------------------------------------------------------------
void toplevel_init_data_list(void) {
  
}
