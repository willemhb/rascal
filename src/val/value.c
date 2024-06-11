#include "val/value.h"
#include "val/type.h"

#include "util/hash.h"

/* Forward declarations */
hash_t hash_nul(Value x);
hash_t hash_bool(Value x);

/* Globals */
// types
Type NulType = {
  .type      =&TypeType,
  .trace     =true,
  .gray      =true,
  .kind      =DATA_TYPE,
  .builtin   =true,
  .idno      =NUL_TYPE,
  .value_type=NUL,
  .value_size=sizeof(Nul),
  .hash_fn   =hash_nul
};

Type BooleanType = {
  .type      =&TypeType,
  .trace     =true,
  .gray      =true,
  .kind      =DATA_TYPE,
  .builtin   =true,
  .idno      =BOOL_TYPE,
  .value_type=BOOL,
  .value_size=sizeof(Boolean),
  .hash_fn   =hash_bool
};

Type PointerType = {
  .type      =&TypeType,
  .trace     =true,
  .gray      =true,
  .kind      =DATA_TYPE,
  .builtin   =true,
  .idno      =CPTR_TYPE,
  .value_type=CPTR,
  .value_size=sizeof(Pointer)
};

Type FuncPtrType = {
  .type      =&TypeType,
  .trace     =true,
  .gray      =true,
  .kind      =DATA_TYPE,
  .builtin   =true,
  .idno      =FPTR_TYPE,
  .value_type=FPTR,
  .value_size=sizeof(FuncPtr)
};

/* APIs */
// comparison methods
hash_t hash_nul(Value x) {
  (void)x;

  static hash_t nul_hash = 0;

  if ( nul_hash == 0 )
    nul_hash = hash_word(NUL);

  return nul_hash;
}

hash_t hash_bool(Value x) {
  static hash_t true_hash  = 0;
  static hash_t false_hash = 0;

  hash_t output;

  if ( x == TRUE ) {
    if ( true_hash == 0 )
      true_hash = hash_word(TRUE);

    output = true_hash;
  } else {
    if ( false_hash == 0 )
      false_hash = hash_word(FALSE);

    output = false_hash;
  }

  return output;
}

// tagging methods
Value tag_nul(Nul n) {
  (void)n;

  return NUL;
}

Value tag_bool(Boolean b) {
  return b ? TRUE : FALSE;
}

Value tag_glyph(Glyph g) {
  return g | GLYPH;
}

Value tag_small(Small s) {
  return s | SMALL;
}

Value tag_arity(Arity a) {
  return a | ARITY;
}

Value tag_real(Real r) {
  union {
    Real r;
    Value v;
  } u = { .r=r };

  return u.v;
}

Value tag_ptr(Pointer p) {
  return (Value)p | CPTR;
}

Value tag_fptr(FuncPtr f) {
  return (Value)f | FPTR;
}

Value tag_obj(void* p) {
  return (Value)p | OBJECT;
}

// casting methods
Nul     as_nul(Value x) {
  (void)x;

  return NULL;
}

Boolean as_bool(Value x) {
  return x == TRUE;
}

Glyph   as_glyph(Value x) {
  return x & WDATA_BITS;
}

Small   as_small(Value x) {
  return x & WDATA_BITS;
}

Arity   as_arity(Value x) {
  return x & DATA_BITS;
}

Real    as_real(Value x) {
  union {
    Real r;
    Value v;
  } u = { .v=x };

  return u.v;
}

Pointer as_ptr(Value x) {
  return (Pointer)(x & DATA_BITS);
}

FuncPtr as_fptr(Value x) {
  return (FuncPtr)(x & DATA_BITS);
}

Object* val_as_obj(Value x) {
  assert(is_obj(x));

  return (Object*)(x & DATA_BITS);
}

Object* ptr_as_obj(void* p) {
  // assert(p != NULL);

  return p;
}

// type_of methods
Type* type_of_val(Value x) {
  extern Type GlyphType, SmallType, ArityType, RealType;
  
  switch (tag_of(x)) {
    default:     return &RealType;
    case ARITY:  return &ArityType;
    case CPTR:   return &PointerType;
    case FPTR:   return &FuncPtrType;
    case NUL:    return &NulType;
    case BOOL:   return &BooleanType;
    case GLYPH:  return &GlyphType;
    case SMALL:  return &SmallType;
    case OBJECT: return as_obj(x)->type;
  }
}

Type* type_of_obj(void* p) {
  assert(p != NULL);

  return ((Object*)p)->type;
}

// has_type methods
bool  val_has_type(Value x, Type* t) {
  return has_instance(t, type_of(x));
}

bool  obj_has_type(void* x, Type* t) {
  return has_instance(t, type_of(x));
}

// size_of methods
size_t size_of_val(Value x, bool o) {
  if ( o ) {
    assert(is_obj(x));

    return size_of_obj(as_obj(x), o);
  } else {
    return type_of(x)->value_size;
  }
}

size_t size_of_obj(void* x, bool o) {
  assert(x != NULL);
  
  if ( o ) {
    Type* xt = type_of(x);

    if ( xt->sizeof_fn )
      return xt->sizeof_fn(x);

    return xt->object_size;
  } else {
    return sizeof(Object*);
  }
}

// value predicates
bool  is_nul(Value x) {
  return x == NUL;
}

bool  is_bool(Value x) {
  return tag_of(x) == BOOL;
}

bool  is_glyph(Value x) {
  return tag_of(x) == GLYPH;
}

bool  is_cptr(Value x) {
  return tag_of(x) == CPTR;
}

bool  is_fptr(Value x) {
  return tag_of(x) == FPTR;
}

bool  is_obj(Value x) {
  return tag_of(x) == OBJECT;
}
