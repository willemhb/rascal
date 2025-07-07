
/**
 *
 * Rascal lists are based on tries. They are the go-to data structure
 * for storing sequenced data.
 *
 * They're a bit more complicated than linked lists, but
 * they have much better general performance characteristics
 * (at some point there will be a 'cons' type in the standard
 * library for handling the cases where a cons cell is actually
 * what's desired).
 *
 * Obviously this is an early implementation and space utilization
 * needs to be optimized.
 *
 **/

// headers --------------------------------------------------------------------
#include "data/list.h"

#include "lang/base.h"

#include "sys/error.h"
#include "sys/memory.h"

// macros ---------------------------------------------------------------------
#define index_for(x, o)                         \
  generic((x),                                  \
          ListNode*:index_for_node,             \
          size_t:index_for_shift)(x, o)

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
// internal helpers -----------------------------------------------------------
size_t tail_offset(List* list);
size_t tail_size(List* list);
size_t index_for_node(ListNode* node, size_t offset);
size_t index_for_shift(size_t shift, size_t offset);
Expr*  array_for(List* list, size_t offset);

List*     new_list(void);
ListLeaf* new_list_leaf(void);
ListLeaf* mk_list_leaf(Expr* xs);

// expression APIs ------------------------------------------------------------
void print_list(Port* ios, Expr x);
bool egal_lists(Expr x, Expr y);

// object APIs ----------------------------------------------------------------
void trace_list(void* ptr);

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------
size_t tail_size(List* list) {
  return list->
}

size_t tail_offset(List* list) {
  if ( list->arity < HAMT_SIZE )
    return 0;

  return list->arity & ~HAMT_MASK;
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

List* new_list(void) {
  List* out  = mk_obj(EXP_LIST, 0, 0);

  return out;
}

ListLeaf* new_list_leaf(void);
ListLeaf* mk_list_leaf(Expr* xs);

// expression APIs ------------------------------------------------------------
void print_list(Port* ios, Expr x) {
  List* xs = as_list(x);

  pprintf(ios, "[");

  if ( xs->root )
    print_traverse(ios, xs->root, ", ", true);

  
}

bool egal_lists(Expr x, Expr y);


// external -------------------------------------------------------------------
List* as_list_s(char* f, Expr x) {
  require_argtype(f, EXP_LIST, x);

  return as_list(x);
}

List* empty_list(void) {
  return new_list();
}

Expr list_ref(List* xs, size_t n) {
  require(n < xs->arity, "List reference '%zu' out of bounds", n);

  Expr* array = array_for(xs, n);

  return array[n & HAMT_MASK];
}

// initialization -------------------------------------------------------------
void toplevel_init_data_list(void) {
  
}
