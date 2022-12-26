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
typedef struct type_t *type_t;

/* vm function pointer types */
typedef val_t  (*native_fn_t)(size_t n, val_t *args);
typedef int    (*guard_fn_t)(size_t n, val_t *args);

typedef void   (*prin_fn_t)(val_t val);
typedef bool   (*isa_fn_t)(type_t self, val_t val);

typedef obj_t  (*create_fn_t)(type_t type, size_t n, void *ini);
typedef void   (*init_fn_t)(obj_t self, type_t type, size_t n, void *ini);
typedef void   (*runtime_fn_t)(obj_t self);
typedef obj_t  (*resize_fn_t)(obj_t self, size_t n);
typedef size_t (*pad_fn_t)(size_t new_count, size_t old_count, size_t old_cap);

/* basic APIs */
extern type_t val_type_of(val_t val);
extern type_t obj_type_of(obj_t obj);
extern bool   val_has_type(val_t val, type_t type);
extern bool   obj_has_type(obj_t obj, type_t type);

#define type_of(x)                              \
  _Generic((x),                                 \
           val_t:val_type_of,                   \
           obj_t:obj_type_of)(x)

#define has_type(x, t)                          \
  _Generic((x),                                 \
           val_t:val_has_type,                  \
           obj_t:obj_has_type)(x, t)

#endif
