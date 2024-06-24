#include "val/type.h"
#include "val/table.h"
#include "val/environ.h"
#include "val/text.h"

#include "util/hash.h"

/* Internal hashing utilities (mostly for circumventing unnecessary dispatch). */
hash_t get_type_hash(Type* type) {
  if ( type->hash == 0 )
    type->hash = hash_word(type->idno);

  return type->hash;
}

bool  has_instance(Type* p, Type* t) {
  Kind k = p->kind;
  bool out;

  switch ( k ) {
    case BOTTOM_TYPE:
      out = false;
      break;
    case DATA_TYPE:
      out = p == t;
      break;
    case UNION_TYPE:
      out = map_has(p->members, tag(t));
      break;
    case TOP_TYPE:
      out = true;
      break;
  }

  return out;
}

char* t_name(Type* x) {
  return x->name->name->chars;
}

Type* get_union_type(size_t n, Type* ts);
void  init_builtin_type(Type* t, char* name, rl_native_fn_t ctor);
