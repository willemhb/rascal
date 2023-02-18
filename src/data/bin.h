#ifndef bin_h
#define bin_h

#include "base/object.h"
#include "base/type.h"

#include "util/ctype.h"

/* C types */
typedef enum BinFl BinFl;

enum BinFl {
  ENCODED=0b00000100000
};

struct Bin {
  Obj obj;

  void* array;
  ushort count, cap;
};

/* globals */
extern Bin EmptyString;

/* API */
bool is_bin(Val x);
Bin* as_bin(Val x);
Val  mk_bin(Bin* x);

// metadata ------------------------------------------------------------------
Ctype bin_ctype(Bin* b);
bool  is_string(Bin* b);
usize bin_el_size(Bin* b);
usize bin_arr_size(Bin* b);
usize bin_used_size(Bin* b);
usize bin_alloc_size(Bin* b);

// constructors & memory management -------------------------------------------
Bin*  new_bin(Ctype type, usize n, void* data, bool mutp);
void  init_bin(Bin* self, Ctype type, usize n, void* data, bool mutp);
void  resize_bin(Bin* self, usize n);

// accessors & mutators -------------------------------------------------------
void* bin_ref(Bin* self, int i);
void* bin_set(Bin* self, int i, void* data);
usize bin_write(Bin* self, usize n, void* data);

// initialization -------------------------------------------------------------
void bin_init(void);

#endif
