#include "util/hashing.h"

#include "lang/envt.h"

#include "val/symbol.h"
#include "val/type.h"

/* globals */
size_t TypeCounter = 0;

/* External API */
Kind get_kind(Type* type) {
  return type->obj.flags;
}

bool is_instance(Type* vt, Type* type) {
  switch (get_kind(type)) {
    case BOTTOM_KIND:
      return false;
      
    case DATA_KIND:
      return vt == type;
      
    case DATA_UNION_KIND: {
        MutSet* members = type->members;
        
        return mset_has(members, tag(vt));
      }
      
    case ABSTRACT_KIND: {
      bool out = false;

      for (;!out && vt != &AnyType; vt=vt->parent)
        out = vt == type;

      return out;
    }

    case ABSTRACT_UNION_KIND: {
      MutSet* members = type->members;
      bool out = false;

      for (;!out && vt != &AnyType; vt=vt->parent)
        out = mset_has(members, tag(vt));

      return out;
    }

    case TOP_KIND:
      return true;
  }

  unreachable();
}

void init_builtin_data_type(Type* type) {
  intern_sym(type->name);
  type->idno = ++TypeCounter;
  Binding* b = defval(type->name, NULL);
  b->value   = tag(type);
}
