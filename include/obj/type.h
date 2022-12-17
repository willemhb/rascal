#ifndef rl_obj_type_h
#define rl_obj_type_h

#include "def/vmtypes.h"

#include "vm/object.h"
#include "vm/value.h"

/* commentary

   First-class type objects used in the rascal type system.

   Some general notes:

   - *Every* type has a constructor.

   - The default action of a constructor is to take a single argument and
     return that value if it's a member of the given type, and otherwise to signal
     an error.

   - Though types are first class values, they are typically represented by their
     constructors.

   - type form returns the constructor rather than the type itself

   - Anonymous types are supported.

   - Non-anonymous union types defined through, eg, (type foo [x, y, z]), are aliases of an
     interned anonymous type. So the above is equivalent to (type foo (type [x, y, z])).
     Hopefully this will facilitate faster comparison.

   - Types are hashed as follows:
     - root  - hash(self.type_kind)
     - primitive - root ^ hash(self.member_object_type) ^ hash(self.member_value_type)
     - union - root ^ hash(self.left) ^ hash(self.right)
     - record - root ^ foldl(^, map(hash, self.namespace))
     - alias - root ^ hash(self.name) ^ hash(self.aliased)

   - All of these hashes are reasonably easy to compute without  
*/

/* C types */
#define RL_TYPE_HEADER				\
  RL_OBJ_HEADER;				\
  rl_string_t *name;				\
  ulong idno, hash;				\
  rl_function_t *constructor

struct rl_type_t {
  RL_TYPE_HEADER;
};

struct rl_alias_type_t {
  RL_TYPE_HEADER;
  rl_type_t *aliased; /* the type for which this is a synonym */
  rl_type_t *base; /* if aliased is itself an alias, this points to the (non-aliased) root type. */
};

struct rl_union_type_t {
  RL_TYPE_HEADER;

  /* unions represented as an invasive binary tree of member types. Member types ordered
     by id number.

     If the union contains more than two members these are simply stored in another union. */

  rl_type_t *left;
  rl_type_t *right;
};

#define RL_DATA_TYPE_HEADER			\
  RL_TYPE_HEADER;				\
  value_type_t value_type;			\
  object_type_t object_type;			\
  size_t value_size;				\
  size_t object_size;				\
  void (*init)(rl_object_t *self, rl_type_t *type, size_t size, bool is_alloc);	\
  void (*trace)(rl_object_t *self);					\
  void (*free)(rl_object_t *self)

struct rl_data_type_t {
  RL_DATA_TYPE_HEADER;
};

struct rl_record_type_t {
  RL_DATA_TYPE_HEADER;
  rl_namespace_t *slots;
};


/* globals */
extern rl_type_t NoneType, AnyType;

/* API */

/* runtime */
void rl_obj_type_init( void );
void rl_obj_type_mark( void );
void rl_obj_type_cleanup( void );

/* convenience */
#define is_datatype( x )  (rl_typeof(x)==&DataType)
#define is_uniontype( x ) (rl_typeof(x)==&UnionType)
#define as_datatype( x )  ((datatype_t*)((x)&PTRMASK))

#define gl_datatype_head  obj_init(&DataType, sizeof(datatype_t), object_fl_static)
#define gl_uniontype_head obj_init(&UnionType, sizeof(uniontype_t), object_fl_static)

#define gl_init_type( x ) init_object(&(x).type.obj)
#define gl_mark_type( x ) mark_object(&(x).type.obj)
#define gl_free_type( x ) free_object(&(x).type.obj)

#endif
