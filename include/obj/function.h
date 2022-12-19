#ifndef rl_obj_function_h
#define rl_obj_function_h

#include "rascal.h"

#include "def/opcodes.h"

#include "vm/obj.h"

#include "vm/obj/support/vector.h"
#include "vm/obj/support/string.h"


/* commentary */

/* C types */
typedef struct metht_t metht_t;
typedef struct metht_cache_t metht_cache_t;
typedef struct metht_node_t metht_node_t;
typedef struct sig_t sig_t;

typedef enum func_fl_t func_fl_t;

enum func_fl_t {
  primp=0,
  nativep=1,
  modulep=2,
  genericp=3,
  effectp=4,
  singletonp=8,
  
};

/* a builtin VM function */
struct prim_t {
  OBJ;

  opcode_t label;
};

/* a builtin C function */
struct native_t {
  OBJ;

  val_t (*callback)(val_t *args, size_t n);
};

/* a reified continuation */
struct cntl_t {
  OBJ;

  vector_t frames;     /* slice from frames */
  vector_t envt;       /* slice from envt */
  vector_t stack;      /* slice from stack */
};

struct func_t {
  OBJ;

  string_t name;  /* name of this function (for printing purposes) */
  type_t *type; /* type for which this function is the constructor (if any) */

  obj_t *methods; /* typically a method table, but might be a leaf method (if singleton)
		     or a simple array of */
};

struct metht_t {
  OBJ;

  metht_cache_t   *cache;     // cache of exact signature matches (crucial for fast dispatch)

  size_t  fargc_min, fargc_max, vargc_min, vargc_max;
  ulong  *fargcs, *vargcs;
  metht_node_t **fmethods, **vmethods;
};

struct metht_node_t {
  OBJ;

  size_t  arity;       // which input to check
  type_t *type;        // type to check against

  size_t n_children;

  metht_node_t  *back; // backtrack when next and down exhausted
  metht_node_t  *next; // try next if match fails
  metht_node_t **down; // 
};

/* globals */

/* API */

/* runtime dispatch */
void rl_obj_function_init( void );
void rl_obj_function_mark( void );
void rl_obj_function_cleanup( void );

/* convenience */

#endif
