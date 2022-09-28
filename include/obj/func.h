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
typedef struct meths_t   meths_t;

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

struct func_t
{
  OBJ_HEAD

  str_t   *name;
  obj_t   *module;
  obj_t   *type;
  meths_t *methods;
};


// forward declarations
// convenience
#define as_func(val) ((func_t*)as_obj(val))
#define is_func(val) is_obj_type(val, func_type)

#endif
