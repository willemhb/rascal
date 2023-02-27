#ifndef value_h
#define value_h

#include "common.h"

// C types --------------------------------------------------------------------
typedef uintptr_t  value_t;
typedef double     real_t;

typedef struct object_t object_t;
typedef struct symbol_t symbol_t;

typedef enum {
  UNIT=1,
  REAL,
  SYMBOL
} type_t;

typedef enum {
  HASHED=1
} objfl_t;

struct object_t {
  object_t *next;
  uword     hash  : 48;
  uword     flags :  9;
  uword     gray  :  1;
  uword     black :  1;
  uword     type  :  5;
};

#define HEADER object_t obj

struct symbol_t {
  HEADER;
  symbol_t *left, *right;
  char*     name;
  value_t   bind;
  uword     idno;
};

// globals --------------------------------------------------------------------
#define QNAN        0x7ff8000000000000ul
#define NUL         0x7ffc000000000000ul
#define OBJTAG      0x7ffd000000000000ul

#define TAG_MASK    0xffff000000000000ul
#define VAL_MASK    0x0000fffffffffffful

#define tag_of(x)   ((x)&TAG_MASK)
#define val_of(x)   ((x)&VAL_MASK)

// API ------------------------------------------------------------------------
// tags, tagging, types -------------------------------------------------------
type_t  type_of(value_t val);
char*   type_name_of(value_t val);

bool    is_real(value_t val);
bool    is_object(value_t val);

value_t tag_ptr(void* p, uword t);
value_t tag_word(uword w, uword t);
value_t tag_dbl(double dbl);

uword   as_word(value_t val);
real_t  as_real(value_t val);
void*   as_ptr(value_t val);

#define as_object(x) ((object_t*)as_ptr(x))
#define as_symbol(x) ((symbol_t*)as_ptr(x))

// constructors ---------------------------------------------------------------
value_t symbol(char* name);

#endif
