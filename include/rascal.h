#ifndef rascal_rascal_h
#define rascal_rascal_h

#include "common.h"

/* commentary

   basic type definitions for the core rascal types. */

/* C types */
/* value types */
typedef uword           val_t;  // unitype
typedef double          real_t;
typedef uchar          *obj_t;

/* object types */
typedef struct sym_t  sym_t;
typedef struct cons_t cons_t;

/* internal types */
typedef struct reader_t reader_t;

/* type code types */
typedef enum val_type_t val_type_t;
typedef enum obj_type_t obj_type_t;
typedef enum rl_type_t  rl_type_t;

/* */
enum val_type_t {
  real_val,
  obj_val,
};

enum obj_type_t {
  nul_obj=obj_val,
  sym_obj,
  cons_obj
};

enum rl_type_t  {
  real_type,
  nul_type,
  sym_type,
  cons_type
};

#define num_types (cons_type+1)

/* internal function pointer types */
typedef size_t (*pad_array_size_fn_t)(size_t new_count, size_t old_count, size_t old_cap);

#endif
