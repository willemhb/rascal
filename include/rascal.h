#ifndef rascal_rascal_h
#define rascal_rascal_h

#include "common.h"

/* commentary

   basic type definitions for the core rascal types. */

/* C types */
/* value types */
typedef uword val_t;  // unitype
typedef double real_t;
typedef uchar *obj_t;

/* object types */
typedef char *sym_t;
typedef struct native_t *native_t;
typedef struct prim_t *prim_t;
typedef struct module_t *module_t;
typedef struct cons_t *cons_t;
typedef val_t *vec_t;
typedef ushort *code_t;

/* internal types */
typedef struct reader_t reader_t;
typedef struct vm_t vm_t;
typedef struct dtype_t *dtype_t;

/* type code types */
typedef enum val_type_t val_type_t;
typedef enum obj_type_t obj_type_t;
typedef enum rl_type_t  rl_type_t;

/* type codes */
enum val_type_t {
  real_val,
  obj_val,
};

enum obj_type_t {
  nul_obj=obj_val,
  sym_obj,
  native_obj,
  prim_obj,
  module_obj,
  cons_obj,
  vec_obj,
  code_obj
};

enum rl_type_t  {
  real_type,
  nul_type,
  sym_type,
  native_type,
  prim_type,
  module_type,
  cons_type,
  vec_type,
  code_type
};

#define num_types (module_type+1)

/* vm function pointer types */
typedef val_t  (*native_fn_t)(size_t nargs, val_t *args);
typedef size_t (*pad_fn_t)(size_t new_count, size_t old_count, size_t old_cap);
typedef void   (*prin_fn_t)(val_t val);
typedef obj_t  (*create_fn_t)(obj_type_t type, size_t n, void *ini);
typedef obj_t  (*resize_fn_t)(obj_t self, size_t n);
typedef void   (*init_fn_t)(obj_t self, obj_type_t type, size_t n, void *ini);
typedef size_t (*objsize_fn_t)(obj_t self);
typedef void   (*runtime_fn_t)(obj_t self);

#endif
