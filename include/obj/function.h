#ifndef rl_obj_function_h
#define rl_obj_function_h

#include "rascal.h"

#include "vm/object.h"

/* commentary

   a rascal function.

   as in julia, all rascal functions are generic.

   functions are also the common way of referring to types. */

/* C types */
typedef struct method_t method_t;
typedef struct method_signature_t method_signature_t;
typedef enum method_type_t method_type_t;
typedef struct method_cache_t method_cache_t;

enum method_type_t {
    invalid_method_type=-1,
    primitive_method_type,
    native_method_type,
    user_method_type,
  };

struct rl_function_t {
  RL_OBJ_HEADER;

  rl_string_t *name;
  rl_type_t *type;
  method_t *methods;
};

struct method_signature_t {
  uint arity;
  bool allow_vargs;
  ulong  hash;
  rl_type_t *return_type;
  rl_type_t **arg_types;
};

struct method_cache_t {
  /* for now cache is a doubly linked list that uses the "most recent to front"
     caching strategy. Amortized, this orders applicable methods by frequency of
     call. */

  method_cache_t *next;
  method_cache_t *prev;
  method_t       *method;
  method_t       *table;
};

struct method_t
{
  method_t *next;

  method_cache_t *cache;
  method_signature_t *signature;
  rl_value_t method_handler;
};

/* globals */

/* API */

/* runtime dispatch */
void rl_obj_function_init( void );
void rl_obj_function_mark( void );
void rl_obj_function_cleanup( void );

/* convenience */

#endif
