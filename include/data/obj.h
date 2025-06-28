#ifndef rl_data_obj_h
#define rl_data_obj_h

/* Common types and APIs for object types. */

// headers --------------------------------------------------------------------
#include "common.h"

#include "data/expr.h"

// macros ---------------------------------------------------------------------
#define HEAD                                     \
  Obj* heap;                                     \
  ExpType type;                                  \
  union {                                        \
    flags_t bfields;                             \
    struct {                                     \
      flags_t black   :   1;                     \
      flags_t gray    :   1;                     \
      flags_t nosweep :   1;                     \
      flags_t flags    : 29;                     \
    };                                           \
  }

#define head(x)        ((Obj*)as_obj(x))

// C types --------------------------------------------------------------------
typedef enum {
  FL_BLACK   = 0x80000000,
  FL_GRAY    = 0x40000000,
  FL_NOSWEEP = 0x20000000
} ExpFlags;

struct Obj {
  HEAD;
};

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
void* as_obj(Expr x);
Expr  tag_obj(void* ptr);
void* mk_obj(ExpType type, flags_t flags);
void* clone_obj(void* ptr);
void  mark_obj(void* ptr);
void  unmark_obj(void* ptr);
void  free_obj(void *ptr);

#endif
