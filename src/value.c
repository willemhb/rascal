#include <string.h>

#include "value.h"
#include "lang.h"
#include "interpreter.h"
#include "runtime.h"
#include "util.h"

/* Globals */
// empty singletons
String EmptyString = {
  .type    =&StringType,
  .meta    =&EmptyMap,

  .trace   =true,
  .gray    =true,
  .encoding=UTF8,

  .chars   ="",
  .count   =0
};

List EmptyList = {
  .type =&ListType,
  .meta =&EmptyMap,

  .trace=true,
  .gray =true,

  .head =NUL,
  .tail =&EmptyList,
  .count=0
};

Vector EmptyVector = {
  .type =&VectorType,
  .meta =&EmptyMap,

  .trace=true,
  .gray =true,

  .root =NULL,
  .count=0,
  .tail ={}
};

Map EmptyMap = {
  .type =&MapType,
  .meta =&EmptyMap,

  .trace=true,
  .gray =true,

  .root =NULL,
  .count=0
};

// standard streams
Port StdIn = {
  .type    =&PortType,
  .meta    =&EmptyMap,

  .trace   =true,
  .gray    =true,
  .encoding=UTF8,
  .input   =true,
  .lispfile=true,

  .ios     =NULL
};

Port StdOut = {
  .type    =&PortType,
  .meta    =&EmptyMap,

  .trace   =true,
  .gray    =true,
  .encoding=UTF8,
  .output  =true,
  .lispfile=true,

  .ios     =NULL
};

Port StdErr = {
  .type    =&PortType,
  .meta    =&EmptyMap,

  .trace   =true,
  .gray    =true,
  .encoding=UTF8,
  .output  =true,

  .ios     =NULL
};

void init_standard_streams(void) {
  StdIn.ios  = stdin;
  StdOut.ios = stdout;
  StdErr.ios = stderr;
}

// type objects
hash_t hash_nul(Value x) {
  static hash_t output = 0;
  
  if ( output == 0 )
    output = hash_word(x);

  return output;
}

Type NulType = {
  .type      =&TypeType,
  .meta      =&EmptyMap,

  .trace     =true,
  .gray      =true,
  .kind      =DATA_TYPE,
  .builtin   =true,

  .idno      =1,
  .value_type=NUL,
  .value_size=sizeof(Nul),

  .hash_fn   =hash_nul
};

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


struct Type BooleanType = {
  .type =&TypeType,
  .meta =&EmptyMap,
  
  .trace  =true,
  .gray   =true,
  .kind   =DATA_TYPE,
  .builtin=true,

  .idno      =2,
  .value_type=BOOL,
  .value_size=sizeof(Boolean),

  .hash_fn   =hash_bool
};

struct Type GlyphType = {
  .type =&TypeType,
  .meta =&EmptyMap,
  
  .trace  =true,
  .gray   =true,
  .kind   =DATA_TYPE,
  .builtin=true,

  .idno      =3,
  .value_type=GLYPH,
  .value_size=sizeof(Glyph)
};

int order_small(Value x, Value y) {
  return as_small(x) - as_small(y);
}

struct Type SmallType = {
  .type =&TypeType,
  .meta =&EmptyMap,
  
  .trace  =true,
  .gray   =true,
  .kind   =DATA_TYPE,
  .builtin=true,

  .idno      =4,
  .value_type=SMALL,
  .value_size=sizeof(Small),

  .order_fn=order_small
};

int order_real(Value x, Value y) {
  return 0 - (as_real(x) < as_real(y)) + (as_real(x) > as_real(y));
}

struct Type RealType = {
  .type =&TypeType,
  .meta =&EmptyMap,
  
  .trace  =true,
  .gray   =true,
  .kind   =DATA_TYPE,
  .builtin=true,

  .idno      =5,
  .value_type=REAL,
  .value_size=sizeof(Real),

  .order_fn=order_real
};


struct Type PointerType = {
  .type =&TypeType,
  .meta =&EmptyMap,
  
  .trace  =true,
  .gray   =true,
  .kind   =DATA_TYPE,
  .builtin=true,

  .idno      =6,
  .value_type=CPTR,
  .value_size=sizeof(Pointer)
};

struct Type FuncPtrType = {
  .type =&TypeType,
  .meta =&EmptyMap,
  
  .trace  =true,
  .gray   =true,
  .kind   =DATA_TYPE,
  .builtin=true,

  .idno      =7,
  .value_type=FPTR,
  .value_size=sizeof(FuncPtr)
};


// object types
// generic hash methods
hash_t hash_mutable(Value x) {
  Object* o = as_obj(x);

  if ( o->hash == 0 )
    o->hash = hash_word(x);

  return o->hash;
}

hash_t hash_type(Value x) {
  Type* t = (Type*)as_obj(x);

  if ( t->hash == 0 )
    t->hash = hash_word(t->idno);

  return t->hash;
}

struct Type TypeType = {
  .type =&TypeType,
  .meta =&EmptyMap,
  
  .trace  =true,
  .gray   =true,
  .kind   =DATA_TYPE,
  .builtin=true,

  .idno       =8,
  .value_type =OBJECT,
  .value_size =sizeof(Type*),
  .object_size=sizeof(Type),
};


struct Type BottomType = {
  .type =&TypeType,
  .meta =&EmptyMap,

  .trace  =true,
  .gray   =true,
  .kind   =BOTTOM_TYPE,
  .builtin=true,

  .idno   =100
};

struct Type TopType = {
  .type =&TypeType,
  .meta =&EmptyMap,

  .trace  =true,
  .gray   =true,
  .kind   =TOP_TYPE,
  .builtin=true,

  .idno   =101
};

void init_types(void) {
  /* Add builtin type objects to global environment. */
  extern Environ Globals;

  define("Nul", tag(&NulType), &Globals);
  define("Boolean", tag(&BooleanType), &Globals);
  define("Glyph", tag(&GlyphType), &Globals);

  define("Small", tag(&SmallType), &Globals);
  define("Real", tag(&RealType), &Globals);

  define("Pointer", tag(&PointerType), &Globals);
  define("FuncPtr", tag(&FuncPtrType), &Globals);

  define("Type", tag(&TypeType), &Globals);

  define("Symbol", tag(&SymbolType), &Globals);

  define("Port", tag(&PortType), &Globals);

  define("Pair", tag(&PairType), &Globals);
  define("List", tag(&ListType), &Globals);

  define("Nothing", tag(&BottomType), &Globals);
  define("Any", tag(&TopType), &Globals);
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
  return (Value)g | GLYPH;
}

Value tag_small(Small s) {
  return (Value)s | SMALL;
}

Value tag_real(Real r) {
  union {
    double d;
    Value  v;
  } u = { .d=r };

  return u.v;
}

Value tag_ptr(Pointer p) {
  assert(p != NULL);

  return (Value)p | CPTR;
}

Value tag_fptr(FuncPtr f) {
  return (Value)f | FPTR;
}

Value tag_obj(void* p) {
  return (Value)p | OBJECT;
}

// object cast methods
Object* val_as_obj(Value v) {
  assert(tag_of(v) == OBJECT);

  return ptr_as_obj((Pointer*)untag(v));
}

Object* ptr_as_obj(void* p) {
  assert(p != NULL);

  return p;
}

// type_of methods
Type* type_of_val(Value v) {
  switch ( tag_of(v) ) {
    default:       return &RealType;
    case NUL:      return &NulType;
    case BOOL:     return &BooleanType;
    case GLYPH:    return &GlyphType;
    case SMALL:    return &SmallType;
    case CPTR:     return &PointerType;
    case FPTR:     return &FuncPtrType;
    case OBJECT:   return as_obj(v)->type;
    case SENTINEL:
      unreachable();
  }
}

Type* type_of_obj(void* p) {
  return as_obj(p)->type;
}

// has_type methods
bool val_has_type(Value v, Type* t) {
  bool out;
  
  switch ( t->kind ) {
    case BOTTOM_TYPE: out = false; break;
    case TOP_TYPE:    out = true;  break;
    case DATA_TYPE:   out = type_of(v) == t; break;
  }

  return out;
}

bool obj_has_type(void* p, Type* t) {
  bool out;
  
  switch ( t->kind ) {
    case BOTTOM_TYPE: out = false; break;
    case TOP_TYPE:    out = true;  break;
    case DATA_TYPE:   out = type_of(p) == t; break;
  }

  return out;
}

/* Value APIs */
// symbols
// qualify methods
Symbol* cstr_qualify(Symbol* s, char* cstr) {
  return mk_sym(get_str(cstr), s->name, is_gs(s));
}

Symbol* str_qualify(Symbol* s, String* str) {
  return mk_sym(str, s->name, is_gs(s));
}

Symbol* sym_qualify(Symbol* s, Symbol* ns) {
  return mk_sym(ns->name, s->name, is_gs(s));
}

// vectors

// maps and sets
static const size_t HashLimit = 16; //

