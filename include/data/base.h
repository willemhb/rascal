#ifndef rl_data_base_h
#define rl_data_base_h

/* Globals, macros, and core APIs for Rascal data.

   Includes templates for implementing simple dynamic array and table types. */

// headers --------------------------------------------------------------------
#include "common.h"

// macros ---------------------------------------------------------------------
// Expression tags 
#define QNAN    0x7ffc000000000000ul
#define NUL_T   0x7ffd000000000000ul
#define BOOL_T  0x7ffe000000000000ul
#define OBJ_T   0x7fff000000000000ul

// variant tags (used internally for various purposes)
#define FIX_T   0xfffc000000000000ul
#define SMALL_T 0xfffd000000000000ul
#define PTR_T   0xfffc000000000000ul
#define NONE_T  0xffff000000000000ul

#define NOTHING NONE_T

// masks
#define XTMSK  0xffff000000000000ul
#define XVMSK  0x0000fffffffffffful

// HAMT magic numbers
#define HAMT_SIZE      0x40ul // = 64
#define HAMT_SHIFT     0x06ul // = 6 (duh-doi)
#define HAMT_MASK      0x3ful // = 63 = 0b00111111
#define HAMT_MAX_SHIFT 0x30ul // = 48
#define HAMT_MAX_DEPTH 0x08ul // = 8 (duh-doi)

// utility macros
#define exp_tag(x) ((x) & XTMSK)
#define exp_val(x) ((x) & XVMSK)

#define exp_type(x)    generic((x), Expr: get_exp_type, default: get_obj_type)(x)
#define has_type(x,t)  (exp_type(x) == (t))

// common object head macro
#define HEAD                                            \
  Obj*  gclist;                                         \
  Type* type;                                           \
  union {                                               \
    uptr_t bit_fields;                                  \
    struct {                                            \
      uptr_t black   :  1;                              \
      uptr_t gray    :  1;                              \
      uptr_t nosweep :  1;                              \
      uptr_t sealed  :  1; /* */                        \
      uptr_t flags   : 12; /* miscellaneous flags */    \
      uptr_t wflags  : 48; /* usually a cached hash */  \
    };                                                  \
  }

#define head(x)   ((Obj*)as_obj(x))
#define is_obj(x) (exp_tag(x) == OBJ_T)

// C types --------------------------------------------------------------------
// memory flags
typedef enum {
  FL_BLACK   = 0x80000000,
  FL_GRAY    = 0x40000000,
  FL_NOSWEEP = 0x20000000,
  FL_FROZEN  = 0x10000000,
} ExpFlags;

// Common Object head type
struct Obj {
  HEAD;
};

// function prototypes --------------------------------------------------------
// expression API -------------------------------------------------------------
Type* get_exp_type(Expr x);
Type* get_obj_type(void* p);
ObjAPI* obj_api(void* ptr);
void mark_exp(Expr x);

// object API -----------------------------------------------------------------
void* as_obj(Expr x);
Expr tag_obj(void* ptr);
void* mk_obj(Type* type, flags_t flags, size_t n);
size_t obj_size(void* ptr);
void* clone_obj(void* ptr);
void mark_obj(void* ptr);
void unmark_obj(void* ptr);
void free_obj(void *ptr);

void* freeze(void* ptr, bool deep);
void* unfreeze(void* ptr, bool deep);

// initialization -------------------------------------------------------------

#endif
