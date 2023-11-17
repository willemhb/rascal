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

typedef struct Vtable {
  size_t     vsize;      // size of the tagged value
  size_t     osize;      // base object size
  uintptr_t  tag;        // tag used for values of the given type

  // lifetime & memory methods
  SizeFn     sizefn;
  TraceFn    tracefn;
  FinalizeFn finalizefn;
  CloneFn    clonefn;

  // comparison & hashing methods
  EgalFn     egalfn;
  HashFn     hashfn;
  RankFn     rankfn;
  OrdFn      ordfn;
} Vtable;

struct Type {
  HEADER;
  idno_t  idno;    // unique identifier determined when object is created
  Type*   parent;  // abstract parent
  Symbol* name;    // common name
  Func*   ctor;    // constructor for this type
  Set*    members; // union members (if any)
  Vtable* vtable;  // instance data
};

/* globals */
// fucked up types and TypeType
extern struct Type NoneType, AnyType, TermType, TypeType;

/* external API */
#define INIT_OBJECT_TYPE(_Type, methods...)                           \
  Symbol _Type##Symbol = {                                            \
    .obj={                                                            \
      .type =&SymbolType,                                             \
      .meta =&EmptyDict,                                              \
      .memfl=NOSWEEP|NOFREE|GRAY,                                     \
    },                                                                \
    .name =#_Type,                                                    \
  };                                                                  \
                                                                      \
  Func _Type##Ctor = {                                                \
    .obj={                                                            \
      .type =&FuncType,                                               \
      .meta =&EmptyDict,                                              \
      .memfl=NOSWEEP|GRAY,                                            \
    },                                                                \
    .name=&_Type##Symbol,                                             \
  };                                                                  \
                                                                      \
  Vtable _Type##Vtable = {                                            \
    .vsize     =sizeof(_Type*),                                       \
    .osize     =sizeof(_Type),                                        \
    .tag       =OBJ_TAG,                                              \
    methods                                                           \
  };                                                                  \
                                                                      \
  Type _Type##Type = {                                                \
    .obj={                                                            \
      .type =&TypeType,                                               \
      .meta =&EmptyDict,                                              \
      .flags=DATA_KIND,                                               \
      .memfl=NOSWEEP|NOFREE|GRAY,                                     \
    },                                                                \
    .parent=&TermType,                                                \
    .name  =&_Type##Symbol,                                           \
    .ctor  =&_Type##Ctor,                                             \
    .vtable=&_Type##Vtable,                                           \
  }

Kind get_kind(Type* type);
bool is_instance(Type* vt, Type* type);
void init_builtin_data_type(Type* type);

#endif
