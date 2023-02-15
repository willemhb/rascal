#include <string.h>

#include "compare.h"
#include "object.h"


/* local helpers */
bool equal_lists(List *x, List *y);
bool equal_pairs(Pair *x, Pair *y);
bool equal_vecs(Vec *x, Vec *y);
bool equal_bins(Bin *x, Bin *y);
bool equal_tables(Table *x, Table *y);
bool equal_ports(Port *x, Port *y);
bool equal_closures(Closure *x, Closure *y);
bool equal_chunks(Chunk *x, Chunk *y);
bool equal_natives(Native *x, Native *y);
bool equal_upvals(UpVal *x, UpVal *y);

int compare_lists(List *x, List *y);
int compare_pairs(Pair *x, Pair *y);
int compare_vecs(Vec *x, Vec *y);
int compare_bins(Bin *x, Bin *y);
int compare_tables(Table *x, Table *y);
int compare_ports(Port *x, Port *y);
int compare_closures(Port *x, Port *y);
int compare_chunks(Chunk *x, Chunk *y);
int compare_natives(Native *x, Native *y);
int compare_upvals(UpVal *x, UpVal *y);

// ----------------------------------------------------------------------------
bool equal_bins(Bin *x, Bin *y) {
  if (x->count != y->count)
    return false;

  if (bin_elsize(x) != bin_elsize(y))
    return false;

  return memcmp(x->array, y->array, x->count * bin_elsize(x));
}


/* API */
bool same(Val x, Val y) {
  /* pointer comparison */
  return x == y;
}

bool equal(Val x, Val y) {
  ValType vx = val_type(x), vy = val_type(y);
  
  if (vx != vy)
    return false;

  if (vx != OBJ_VAL)
    return x == y;

  ObjType ox = obj_type(x), oy = obj_type(y);

  if (ox != oy)
    return false;

  switch (ox) {
  case       LIST_OBJ: return equal_lists(AS_LIST(x), AS_LIST(y));
  case       PAIR_OBJ: return equal_pairs(AS_PAIR(x), AS_PAIR(y));
  case       VEC_OBJ:  return equal_vecs(AS_VEC(x), AS_VEC(y));
  case       
  default:             return x == y;
  }
}
