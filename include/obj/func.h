#ifndef rascal_func_h
#define rascal_func_h

#include "obj.h"
#include "obj/envt.h"
#include "template/table.h"
#include "template/str.h"
#include "template/arr.h"

typedef struct func_t    func_t;
typedef struct cntl_t    cntl_t;
typedef struct str_t     str_t;
typedef struct code_t    code_t;
typedef struct clo_t     clo_t;
typedef struct method_t  method_t;
typedef struct methods_t methods_t;

typedef enum
  {
    // function type discriminator
    func_fl_common  =0x000, // common function with usual semantics
    func_fl_script  =0x001, // function is a module body
    func_fl_macro   =0x002, // a syntax expander
    func_fl_dtype   =0x003,
    func_fl_utype   =0x004,
    func_fl_class   =0x005,
    func_fl_cntl    =0x006,
  } func_fl_t;

typedef enum
  {
    method_fl_none     = 0x000, // no method matching the current signature
    method_fl_primitive= 0x001,
    method_fl_native   = 0x002,
    method_fl_bytecode = 0x003,
    method_fl_vargs    = 0x004,
  } method_fl_t;


DECL_STRING(sig);
STRING_TYPE(sig, EMPTY_HEAD, type_t, type_t, type);

DECL_ALIST(meths);
ALIST_TYPE(meths, EMPTY_HEAD, method_t*);
DECL_ALIST_API(meths, method_t*);

struct methods_t
{
  OBJ_HEAD;
  /* min and max arities used when computing the signature hash */
  arity_t    argmin;
  arity_t    argmax;

  func_t    *func;
  meths_t    cache;
  method_t  *tree;
};

typedef struct method_t
{
  OBJ_HEAD;
  sig_t          sig;
  hash_t         hash;

  union
  {
    opcode_t     primitive;
    native_fn_t  native;
    code_t      *bytecode;
  };

  methods_t *ancestor;
  method_t  *parent;

  // invasive unordered hash table of signatures
  meths_t   children;
} method_t;

struct func_t
{
  OBJ_HEAD;

  str_t     *name;
  obj_t     *module;
  obj_t     *type;
  methods_t *methods;
};

// forward declarations
bool get_method(func_t *func, arity_t n, val_t *args, method_t **buf );
bool put_method(func_t *func, arity_t n, val_t *args, method_t **buf );
bool pop_method(func_t *func, arity_t n, val_t *args, method_t **buf );

// convenience
#define as_func(val) ((func_t*)as_obj(val))
#define is_func(val) is_obj_type(val, func_type)

#define as_method(val) ((method_t*)as_obj(val))
#define is_method(val) is_obj_type(val, method_type)

#endif
