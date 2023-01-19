#ifndef rascal_object_h
#define rascal_object_h

#include "value.h"

/* object alist for internal use */
#include "decl/alist.h"

ALIST(Objects, Object);

/* C types */
struct Object {
  /* next object pointer */
  struct Object *next;

  /* type/value metadata */
  uhash  hash;
  RlType type;

  ushort flags     : 10; // discretionary
  ushort hashed    :  1;

  /* size/layout/memory data */
  ushort lendee    :  1;
  ushort inlined   :  1;
  ushort allocated :  1;
  ushort gray      :  1;
  ushort black     :  1;
  ushort offset; // offset from space to object start (if header is extended)
  usize  size;   // total size of data owned by this object

  /* object data (tagged value points to here) */
  uchar  space[0];
};

struct ObjectInit {
  RlType type;
  uint16 flags     : 12;
  uint16 hashed    :  1;
  uint16 lendee    :  1;
  uint16 inlined   :  1;
  uint16 allocated :  1;
  uint16 offset;
  usize  size;
  usize  hash;

  union {
    struct {
      char  *name;
      int    nArgs;
      uint64 idno; 
    } SymbolInit;

    struct {
      Symbol name;
      RlType type;
    } FunctionInit;

    struct {
      Value *args;
      int    nArgs;
    } ListInit;

    struct {
      Value car;
      Value cdr;
    } PairInit;

    struct {
      Value *array;
      int    nArgs;
      int    capacity;
    } TupleInit;

    struct {
      char *array;
      int   nArgs;
      int   capacity;
    } StringInit;

    struct {
      uint16 *array;
      int     nArgs;
      int     capacity;
    } ByteCodeInit;

    struct {
      NameSpc next;
      List    names;
    } NameSpcInit;

    struct {
      Environ next;
      Tuple   binds;
    } EnvironInit;

    struct {
      Symbol name;
      int    nArgs;
      bool   vArgs;
      Object handler;
    } MethodInit;

    struct {
      Symbol   name;
      NameSpc  names;
      Environ  envt;
      Tuple    consts;
      ByteCode code;
    } UserMethodInit;

    struct {
      Symbol name;
      EvalFn eval;
      CompFn comp;
      ExecFn exec;
    } NativeMethodInit;
  };
};

typedef enum {
  LiteralSymbol =1, // always evaluates to itself
  BoundSymbol   =2, // has toplevel binding
  ConstantSymbol=4, // toplevel binding cannot be rebound
} SymbolFlags;

struct Symbol {
  ulong idno;
  Value bind;        // toplevel binding

  struct Object obj;
  ascii  name[];
};

struct Function {
  Symbol   name;
  RlType   type;         // if the function represents a type then this is the key
  Method   vMethod;      // variadic method
  Objects  methods;
};

struct List {
  List  tail;
  Value head;
  usize length;
};

struct Pair {
  Value cdr;
  Value car;
};

#include "decl/array.h"

ARRAY_OBJECT(Tuple, Value);
ARRAY_OBJECT(String, Glyph);
ARRAY_OBJECT(ByteCode, ushort);

#include "decl/htable.h"

HTABLE(SymbolTable, char*, Symbol);

extern SymbolTable RlSymbolTable;

struct NameSpc {
  NameSpc next;
  List    names;
};

struct Environ {
  Environ next;
  Tuple   binds;
};

struct Method {
  Symbol name;
  int    nargs;
  bool   vargs;
  Object handler; // native or closure
};

struct UserMethod { // a closure
  Symbol   name;
  NameSpc  names;
  Environ  envt;
  Tuple    consts;
  ByteCode code;
};

struct NativeMethod {
  Symbol       name;
  EvalFn       eval;
  CompFn       comp;
  ExecFn       exec;
};

/* convenience */
#define IS_SYM(x)      hasType(x, SymbolType)
#define IS_FUNC(x)     hasType(x, FunctionType)
#define IS_LIST(x)     hasType(x, ListType)
#define IS_PAIR(x)     hasType(x, PairType)
#define IS_TUPLE(x)    hasType(x, TupleType)
#define IS_STRING(x)   hasType(x, StringType)
#define IS_BYTECODE(x) hasType(x, ByteCodeType)
#define IS_NAMESPC(x)  hasType(x, NameSpcType)
#define IS_ENVIRON(x)  hasType(x, EnvironType)
#define IS_METHOD(x)   hasType(x, MethodType)
#define IS_USER(x)     hasType(x, UserMethodType)
#define IS_NATIVE(x)   hasType(x, NativeMethodType)

#define AS_SYM(x)      ((Symbol)AS_OBJ(x))
#define AS_FUNC(x)     ((Function)AS_OBJ(x))
#define AS_LIST(x)     ((List)AS_OBJ(x))
#define AS_PAIR(x)     ((Pair)AS_OBJ(x))
#define AS_TUPLE(x)    ((Tuple)AS_OBJ(x))
#define AS_STRING(x)   ((String)AS_OBJ(x))
#define AS_BYTECODE(x) ((ByteCode)AS_OBJ(x))
#define AS_NAMESPC(x)  ((NameSpc)AS_OBJ(x))
#define AS_USER(x)     ((UserMethod)AS_OBJ(x))
#define AS_NATIVE(x)   ((NativeMethod)AS_OBJ(x))

#define OBJ_HEAD(obj)   (objHead((Object)(obj)))
#define OBJ_START(obj)  (objStart((Object)(obj)))
#define SYM_HEAD(obj)   ((struct Symbol*)OBJ_START(obj))
#define TUPLE_HEAD(obj) ((struct Tuple*)OBJ_START(obj))
#define STR_HEAD(obj)   ((struct String*)OBJ_START(obj))

/* API */
static inline struct Object *objHead(Object obj) {
  return (struct Object*)(obj-sizeof(struct Object));
}

static inline ubyte *objStart(Object obj) {
  return obj - (objHead(obj)->offset);
}

/* symbol flag predicates */
static inline bool isKeyWord(Symbol s) {
  return !!(OBJ_HEAD(s)->flags & LiteralSymbol);
}

static inline bool isBound(Symbol s) {
  return !!(OBJ_HEAD(s)->flags & BoundSymbol);
}

static inline bool isConstant(Symbol s) {
  return !!(OBJ_HEAD(s)->flags & ConstantSymbol);
}

Object constructObject(ObjectInit *args);
Object createObject(ObjectInit *args);
void   destroyObject(Object self, struct Object **next);
void   initObject(Object self, ObjectInit *args);
void   freeObject(Object self, struct Object **next);

Value  symbolToValue(Symbol s);
Value  functionToValue(Function f);
Value  listToValue(List l);
Value  pairToValue(Pair p);
Value  tupleToValue(Tuple t);
Value  stringToValue(String s);
Value  byteCodeToValue(ByteCode b);
Value  nameSpcToValue(NameSpc n);
Value  environToValue(Environ e);
Value  methodToValue(Method m);
Value  nativeMethodToValue(NativeMethod n);
Value  userMethodToValue(UserMethod u);

Symbol       symbol(char *name);
Function     function(char *name, RlType type);
List         list(Value *args, int nArgs);
Pair         pair(Value car, Value cdr);
Tuple        tuple(Value *args, int nArgs);
String       string(char *chars);
ByteCode     bytecode(ushort *args, int nArgs);
NameSpc      namespc(List names, NameSpc next);
Environ      environ(Tuple binds, Environ next);
Method       method(Symbol name, int nArgs, bool vargs, Object handler);
UserMethod   usermethod(Symbol name, NameSpc names, Environ envt, Tuple consts, ByteCode code);
NativeMethod nativemethod(Symbol name, EvalFn eval, CompFn comp, ExecFn exec);

#endif
