#ifndef object_h
#define object_h

#include "value.h"

/* Object lifetime methods, eval types (types with syntactic relevance), and alist/table. */


#define HEADER object_t obj
#define TYPE_HEADER type_t type

// C types --------------------------------------------------------------------
typedef enum {
  HASHED = 0x40,
  FROZEN = 0x80
} obj_fl_t;

struct object_t {
  object_t *next;
  uword hash   : 48;
  uword flags  :  8;
  uword type   :  6;
  uword gray   :  1;
  uword black  :  1;
  ubyte data[0];
};

typedef enum {
  IMMUTABLE = 0x01, // new instances are frozen by default
} type_fl_t;

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
  bool    (*equal)(void* x, void* y);
  int     (*compare)(value_t x, value_t y);

  // lifetime methods ---------------------------------------------------------
  void*   (*alloc)(data_type_t* type, usize count, flags fl);
  void*   (*copy)(void* self, usize padding);
  void    (*init)(void* self, data_type_t* type, usize count, flags fl);
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

struct iterator_t {
  HEADER;
  data_type_t* type;
  object_t*    self;
  usize        statesize;
  void*        state;     // always inlined
};

// globals --------------------------------------------------------------------
extern data_type_t DataTypeType, UnionTypeType, IteratorType;

// API ------------------------------------------------------------------------
// object ---------------------------------------------------------------------
// utilities ------------------------------------------------------------------
#define is_object(x) IST(x, OBJTAG, TAG_MASK)
#define as_object(x) ASP(x, object_t)

#define header(o)    ((object_t*)(o))
#define object(o)    ((((uword)(o)) & VAL_MASK) | OBJTAG)
#define freeze(o)    (((object_t*)(o))->frozen = true)
#define unfreeze(o)  (((object_t*)(o))->frozen = false)

void trace_objects(usize n, object_t** objs);
void trace_values(usize n, value_t* vals);

// lifetime -------------------------------------------------------------------
void*   new_object(data_type_t* type, usize count, flags fl);
void*   alloc_object(data_type_t* type, usize count, flags fl);
void*   clone_object(void* self, usize padding);
void    init_object(void* self, data_type_t* type, usize count, flags fl);
void    mark_object(void* self);
void    free_object(void* self);

// type -----------------------------------------------------------------------

#endif
