#ifndef rl_obj_type_h
#define rl_obj_type_h

#include "rascal.h"

#include "vm/obj.h"
#include "vm/obj/support/sig.h"
#include "vm/obj/support/string.h"

/* commentary */

/* C types */
typedef struct atype_t atype_t;
typedef struct dtype_t dtype_t;
typedef struct utype_t utype_t;

struct type_t {
  OBJ;

  string_t name;
  ulong idno, hash;
  func_t *constructor;
};

struct atype_t {
  type_t type;

  type_t *aliased;     // NB: never another alias type
};

struct dtype_t {
  type_t type;

  val_type_t val_type;
  obj_type_t obj_type;
  size_t     size;
  ns_t      *slots;    // null unless this is a record type
};

struct utype_t {
  type_t type;

  sig_t *members;
};

/* globals */

/* API */

/* runtime dispatch */
void rl_obj_type_init( void );
void rl_obj_type_mark( void );
void rl_obj_type_cleanup( void );

/* convenience */

#endif
