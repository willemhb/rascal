#ifndef rascal_tuple_h
#define rascal_tuple_h

#include "obj/object.h"

struct cons_t
{
  OBJ
  value_t  head;
  cons_t  *tail;
  size_t   length;
};

struct pair_t
{
  OBJ
  value_t car;
  value_t cdr;
  union
  {
    size_t length;
    hash_t hash;
  };
};

struct tuple_t
{
  OBJ
  type_t  *type;    // tuple types
  size_t   length;
  value_t  data[];
};

// association types
typedef struct namespace_t namespace_t;

struct variable_t
{
  OBJ
  value_t      name;
  value_t      bind;            // may or may not contain the actual value
  hash_t       hash;
  namespace_t *namespace;       // namespace in which this variable was defined
  type_t      *type;
  cons_t      *props;
  uintptr_t    offset     : 48;
  uintptr_t    scope      :  2; // local, module, or toplevel
  uintptr_t    funcname   :  1; // function name?
  uintptr_t    typename   :  1; // type name?
  uintptr_t    macroname  :  1; // macro name?
  uintptr_t    modulename :  1; // module name?
  uintptr_t    exported   :  1; // externally visible?
  uintptr_t               :  9;
};

struct slot_t
{
  OBJ
  value_t     key;          // typically a symbol, but could be a fixnum (for unnamed tuple types)
  value_t     value;        // if this slot is an instance slot, this is the default.
  hash_t      hash;
  type_t     *owner;        // type this slot is meaningful for
  type_t     *self;         // type of the slot value
  cons_t     *props;        // metadata
  uintptr_t   offset   : 48; // offset from object head (in bytes)
  uintptr_t   Ctype    :  4; // Ctype of the slot value
  uintptr_t   unboxed  :  1; // whether or not the slot is tagged
  uintptr_t   read     :  1; // can be referenced?
  uintptr_t   write    :  1; // can be modified?
  uintptr_t   instance :  1; // belongs to instance?
  uintptr_t   size     :  4; // size of the slot
  uintptr_t   align    :  4; // alignment of the slot
};

struct upvalue_t
{
  OBJ
  value_t    value;
  upvalue_t *next;           // place in list of open upvalues
  uintptr_t  offset : 48;    // location on the stack
  uintptr_t  closed : 16;    // 
};

// forward declarations

// convenience

#endif
