#include "val/value.h"
#include "val/type.h"

#include "util/hash.h"

/* Forward declarations */
hash_t hash_nul(Val x);
hash_t hash_bool(Val x);

/* Globals */
// types
Type NulType = {
  .type      =&TypeType,
  .trace     =true,
  .gray      =true,
  .kind      =DATA_TYPE,
  .builtin   =true,
  .idno      =NUL_TYPE,
  .val_type  =NUL,
  .val_size  =sizeof(Nul),
  .hash_fn   =hash_nul
};

Type BoolType = {
  .type    =&TypeType,
  .trace   =true,
  .gray    =true,
  .kind    =DATA_TYPE,
  .builtin =true,
  .idno    =BOOL_TYPE,
  .val_type=BOOL,
  .val_size=sizeof(Bool),
  .hash_fn =hash_bool
};

Type PtrType = {
  .type      =&TypeType,
  .trace     =true,
  .gray      =true,
  .kind      =DATA_TYPE,
  .builtin   =true,
  .idno      =CPTR_TYPE,
  .val_type=CPTR,
  .val_size=sizeof(Ptr)
};

Type FunPtrType = {
  .type      =&TypeType,
  .trace     =true,
  .gray      =true,
  .kind      =DATA_TYPE,
  .builtin   =true,
  .idno      =FPTR_TYPE,
  .val_type=FPTR,
  .val_size=sizeof(FunPtr)
};

/* APIs */
// comparison methods
hash_t hash_nul(Val x) {
  (void)x;

  static hash_t nul_hash = 0;

  if ( nul_hash == 0 )
    nul_hash = hash_word(NUL);

  return nul_hash;
}

hash_t hash_bool(Val x) {
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
Val tag_nul(Nul n) {
  (void)n;

  return NUL;
}

Val tag_bool(Bool b) {
  return b ? TRUE : FALSE;
}

Val tag_glyph(Glyph g) {
  return g | GLYPH;
}

Val tag_small(Small s) {
  return s | SMALL;
}

Val tag_arity(Arity a) {
  return a | ARITY;
}

Val tag_real(Real r) {
  union {
    Real r;
    Val v;
  } u = { .r=r };

  return u.v;
}

Val tag_ptr(Ptr p) {
  return (Val)p | CPTR;
}

Val tag_fptr(FunPtr f) {
  return (Val)f | FPTR;
}

Val tag_obj(void* p) {
  return (Val)p | OBJECT;
}

// casting methods
Nul     as_nul(Val x) {
  (void)x;

  return NULL;
}

Bool as_bool(Val x) {
  return x == TRUE;
}

Glyph   as_glyph(Val x) {
  return x & WDATA_BITS;
}

Small   as_small(Val x) {
  return x & WDATA_BITS;
}

Arity   as_arity(Val x) {
  return x & DATA_BITS;
}

Real    as_real(Val x) {
  union {
    Real r;
    Val v;
  } u = { .v=x };

  return u.v;
}

Ptr as_ptr(Val x) {
  return (Ptr)(x & DATA_BITS);
}

FunPtr as_fptr(Val x) {
  return (FunPtr)(x & DATA_BITS);
}

Obj* val_as_obj(Val x) {
  assert(is_obj(x));

  return (Obj*)(x & DATA_BITS);
}

Obj* ptr_as_obj(void* p) {
  // assert(p != NULL);

  return p;
}

// type_of methods
Type* type_of_val(Val x) {
  extern Type GlyphType, SmallType, ArityType, RealType;
  
  switch (tag_of(x)) {
    default:     return &RealType;
    case ARITY:  return &ArityType;
    case CPTR:   return &PtrType;
    case FPTR:   return &FunPtrType;
    case NUL:    return &NulType;
    case BOOL:   return &BoolType;
    case GLYPH:  return &GlyphType;
    case SMALL:  return &SmallType;
    case OBJECT: return as_obj(x)->type;
  }
}

Type* type_of_obj(void* p) {
  assert(p != NULL);

  return ((Obj*)p)->type;
}

// has_type methods
bool  val_has_type(Val x, Type* t) {
  return has_instance(t, type_of(x));
}

bool  obj_has_type(void* x, Type* t) {
  return has_instance(t, type_of(x));
}

// size_of methods
size_t size_of_val(Val x, bool o) {
  if ( o ) {
    assert(is_obj(x));

    return size_of_obj(as_obj(x), o);
  } else {
    return type_of(x)->val_size;
  }
}

size_t size_of_obj(void* x, bool o) {
  assert(x != NULL);
  
  if ( o ) {
    Type* xt = type_of(x);

    if ( xt->sizeof_fn )
      return xt->sizeof_fn(x);

    return xt->obj_size;
  } else {
    return sizeof(Obj*);
  }
}

// value predicates
bool  is_nul(Val x) {
  return x == NUL;
}

bool  is_bool(Val x) {
  return tag_of(x) == BOOL;
}

bool  is_glyph(Val x) {
  return tag_of(x) == GLYPH;
}

bool  is_cptr(Val x) {
  return tag_of(x) == CPTR;
}

bool  is_fptr(Val x) {
  return tag_of(x) == FPTR;
}

bool  is_obj(Val x) {
  return tag_of(x) == OBJECT;
}
