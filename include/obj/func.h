#ifndef rascal_func_h
#define rascal_func_h

#include "obj.h"

typedef struct func_t func_t;
typedef struct str_t  str_t;
typedef struct envt_t envt_t;

typedef enum
  {
    func_fl_vargs =0x0001,
    func_fl_macro =0x0002,

    // these flags indicate the type of the function's template
    func_fl_multi  =0x0004,
    func_fl_code   =0x0008,
    func_fl_native =0x0010,
    func_fl_prim   =0x0020,
    func_fl_script =0x0040,

    // these flags indicate whether a function is a constructor
    func_fl_data   =0x0080,
    func_fl_union  =0x0100,
    func_fl_enum   =0x0200,
    func_fl_class  =0x0400,
  } func_fl_t;

typedef struct sig_t
{
  type_t  *types;
  arity_t  argco;
  bool     vargs;
  hash_t   hash;
} sig_t;

struct func_t
{
  OBJ_HEAD;

  str_t  *name;
  sig_t  *sig;

  val_t   tplt;
};

// convenience
#define as_func(val) ((func_t*)as_obj(val))
#define is_func(val) is_obj_type(val, func_type)

static inline bool is_native(val_t fn)
{
  return is_func(fn)
    && val_tag(as_func(fn)->tplt) == POINTER;
}

static inline bool is_compiled(val_t fn)
{
  
}

#endif
