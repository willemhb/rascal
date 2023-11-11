#ifndef rascal_type_h
#define rascal_type_h

#include "val/table.h"
#include "val/object.h"

/* globals & utilites for working with types. */
/* C types */
typedef enum {
  BOTTOM_KIND, DATA_KIND, DATA_UNION_KIND,
  ABSTRACT_KIND, ABSTRACT_UNION_KIND, TOP_KIND,
} Kind;

typedef void   (*TraceFn)(Obj* slf);
typedef void   (*FreeFn)(Obj* slf);
typedef size_t (*SizeFn)(Obj* slf);

typedef struct Vtable {
  size_t    vsize;    // size of the tagged value
  size_t    osize;    // base object size
  uintptr_t tag;      // tag used for values of given type
  SizeFn    size;     // called to determine size of owned data
  TraceFn   trace;    // called by the GC to trace owned pointers
  FreeFn    finalize; // called by the GC to free managed data
} Vtable;

struct Type {
  HEADER;
  idno_t  idno;    // unique identifier determined when object is created
  Type*   parent;  // abstract parent
  Symbol* name;    // common name
  Func*   ctor;    // constructor for this type
  MutSet* members; // union members (if any)
  Vtable* vtable;  // instance data
};

/* globals */
// fucked up types and TypeType
extern struct Type NoneType, AnyType, TermType, TypeType;

/* external API */
void register_builtin_type(Type* type);

#endif
