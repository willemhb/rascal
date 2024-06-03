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
struct Type NulType = {
  .type      =&TypeType,
  .meta      =&EmptyMap,

  .trace     =true,
  .gray      =true,
  .kind      =DATA_TYPE,
  .builtin   =true,

  .idno      =1,
  .value_type=NUL,
  .value_size=sizeof(Nul),
};

void init_types(void) {
  /* Add builtin type objects to global environment. */
  extern Environ Globals;

  define("Nul", tag(&NulType), &Globals);
  define("Boolean", tag(&BooleanType), &Globals);
  define("Glyph", tag(&GlyphType), &Globals);

  define("Small", tag(&SmallType), &Globals);
  define("Number", tag(&NumberType), &Globals);

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

Value tag_num(Number n) {
  union {
    double d;
    Value  v;
  } u = { .d=n };

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
    default:       return &NumberType;
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
