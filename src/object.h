#ifndef object_h
#define object_h

#include "value.h"

/* Object lifetime methods, eval types (types with syntactic relevance), and alist/table. */


#define HEADER object_t obj
#define TYPE_HEADER type_t type

// C types --------------------------------------------------------------------
struct object_t {
  object_t *next;
  uword hash   : 48;
  uword flags  :  6;
  uword hashed :  1;
  uword frozen :  1;
  uword type   :  6;
  uword gray   :  1;
  uword black  :  1;
  ubyte data[0];
};

typedef enum {
  ITERABLE = 0x01, // first and rest can be called on this object
} obj_fl_t;

struct type_t {
  HEADER;
  char*    name;
  uint64   idno;
  kind_t (*isa)(value_t v, type_t* self);
};

struct data_type_t {
  TYPE_HEADER;
  usize     size;
  dict_t*   slots;

  // vtable -------------------------------------------------------------------
  // sacred methods -----------------------------------------------------------
  void    (*print)(value_t val, port_t* ios);
  uhash   (*hash)(void* x);
  int     (*equal)(void* x, void* y);
  int     (*compare)(value_t x, value_t y);

  // traversal methods --------------------------------------------------------
  void*   (*iter)(void* iterable);
  value_t (*next)(void** iterbuf);
  bool    (*hasnext)(void* iterable);

  // lifetime methods ---------------------------------------------------------
  void*   (*alloc)(type_t* type, usize count, flags fl);
  void*   (*copy)(void* self, usize padding);
  void    (*init)(void* self, usize count, flags fl);
  void    (*trace)(void* self);
  void    (*free)(void* self);

  // misc ---------------------------------------------------------------------
  int     (*write)(type_t* type, value_t val, void* spc);
  usize   (*size_of)(void* ptr);
};

struct union_type_t {
  TYPE_HEADER;

  data_type_t*  leaf;
  union_type_t* left;
  union_type_t* right;
};

// API ------------------------------------------------------------------------
// object ---------------------------------------------------------------------
#define is_object(x) IST(x, OBJTAG, TAG_MASK)
#define as_object(x) ASP(x, object_t)

#define header(o)    ((object_t*)(o))
#define object(o)    ((((uword)(o)) & VAL_MASK) | OBJTAG)
#define freeze(o)    (((object_t*)(o))->frozen = true)
#define unfreeze(o)  (((object_t*)(o))->frozen = false)

void*   new_object(data_type_t* type, usize count, flags fl);
void*   copy_object(void* self, usize padding, bool editable);
void    mark_object(void* self);
void    free_object(void* self);

void*   iter(void* ptr);
value_t next(void** buf);
bool    hasnext(void* ptr);

#endif
