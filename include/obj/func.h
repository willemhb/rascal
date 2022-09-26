#ifndef rascal_func_h
#define rascal_func_h

#include "obj.h"
#include "obj/envt.h"
#include "template/tuple.h"

typedef struct func_t func_t;
typedef struct str_t  str_t;
typedef struct code_t code_t;

typedef uint32_t func_fl_t;

enum
  {
   // general information about the function
   func_fl_vargs    =0x001,
   func_fl_macro    =0x002,

   // indicates the type of the function body
   func_fl_native   =0x004,
   func_fl_primitive=0x008,
   func_fl_bytecode =0x010,
   func_fl_multi    =0x020,

   // indicates the type denoted by the function (if any)
   func_fl_data     =0x040,
   func_fl_enum     =0x080,
   func_fl_union    =0x100,
  };

DECL_SIG(sig, type_t, type);
DECL_SIG_API(sig, type_t, type);

struct func_t
{
  OBJ_HEAD;
  
  str_t     *name;
  arity_t    argco;
  func_fl_t  fl;
  
  union
  {
    native_fn_t native;
    opcode_t    primitive;
    code_t     *code;
  };

  sig_t *sig;
};

// convenience
#define as_func(val) ((func_t*)as_obj(val))
#define is_func(val) is_obj_type(val, func_type)

static inline bool is_native(val_t fn)
{
  return is_func(fn)
    && flag_p(as_func(fn)->fl, func_fl_native);
}

static inline bool is_compiled(val_t fn)
{
  return is_func(fn)
    && flag_p(as_func(fn)->fl, func_fl_bytecode);
}

static inline bool is_multi(val_t fn)
{
  return is_func(fn)
    && flag_p(as_func(fn)->fl, func_fl_multi);
}

#endif
