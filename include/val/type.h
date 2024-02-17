#ifndef rascal_type_h
#define rascal_type_h

#include "val/table.h"
#include "val/object.h"

/* globals & utilites for working with types. */
/* C types */
typedef enum {
  BOTTOM_KIND, DATA_KIND, TOP_KIND,
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
  HashFn     hashfn;
  EgalFn     egalfn;
  RankFn     rankfn;
  OrdFn      ordfn;
} Vtable;

struct Type {
  HEADER;
  idno_t  idno;    // unique identifier determined when object is created
  Kind    kind;    // indicates how to determine type membership
  Symbol* name;    // common name
  Vtable* vtable;  // instance data
};

/* globals */
// fucked up type objects
extern struct Type NoneType, AnyType, TypeType;

/* external API */
#define INIT_OBJECT_TYPE(_Type, methods...)                             \
  String _Type##Name = {                                                \
    .obj={                                                              \
      .type  =&StringType,                                              \
      .meta  =&EmptyDict,                                               \
      .flags =NOSWEEP|NOFREE|GRAY,                                      \
    },                                                                  \
    .chars = #_Type,                                                    \
    .arity = sizeof(#_Type) - 1                                         \
  };                                                                    \
                                                                        \
  Symbol _Type##Symbol = {                                              \
    .obj={                                                              \
      .type =&SymbolType,                                               \
      .meta =&EmptyDict,                                                \
      .flags=NOSWEEP|NOFREE|GRAY,                                       \
    },                                                                  \
    .name =&_Type##Name,                                                \
  };                                                                    \
                                                                        \
  Vtable _Type##Vtable = {                                              \
    .vsize     =sizeof(_Type*),                                         \
    .osize     =sizeof(_Type),                                          \
    .tag       =OBJ_TAG,                                                \
    methods                                                             \
  };                                                                    \
                                                                        \
  Type _Type##Type = {                                                  \
    .obj={                                                              \
      .type =&TypeType,                                                 \
      .meta =&EmptyDict,                                                \
      .flags=NOSWEEP|NOFREE|GRAY,                                       \
    },                                                                  \
    .kind  =DATA_KIND,                                                  \
    .name  =&_Type##Symbol,                                             \
    .vtable=&_Type##Vtable,                                             \
  }

bool is_instance(Type* vt, Type* type);
void init_builtin_data_type(Type* type);

#endif
