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

struct Symbol {
  ulong idno;
  Value bind;        // toplevel binding
  bool  isKeyWord;
  bool  isBound;

  struct Object obj;
  ascii  name[];
};

struct Function {
  Symbol   name;
  RlType   type;         // if the function represents a type then this is the key
  Method   vMethod;      // variadic method
  Objects  Methods;
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

struct Environ {
  Environ next;
  Tuple   binds; // null unless environ represents a runtime closure
};

struct Method {
  int    nargs;
  bool   vargs;
  Object handler; // native or user
};

struct UserMethod {
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

/* API */
static inline struct Object *objHead(Object obj) {
  return (struct Object*)(obj-sizeof(struct Object));
}

static inline ubyte *objStart(Object obj) {
  return obj - (objHead(obj)->offset);
}

// general api
Object createObject(RlType type, usize size);
void   destroyObject(Object self);
void   initObject(Object self, RlType type, usize size);
void   freeObject(Object self);

Value  symbolToValue(Symbol s);
Value  functionToValue(Function f);
Value  listToValue(List l);
Value  pairToValue(Pair p);
Value  tupleToValue(Tuple t);
Value  stringToValue(String s);


Symbol   symbol(char *name);
Function function(char *name, RlType type);
List     list(int nArgs, Value *args);
Pair     pair(Value car, Value cdr);
Tuple    tuple(int nArgs, Value *args);
String   string(char *chars);


/* convenience */
#define IS_SYM(x)      hasType(x, SymbolType)
#define IS_LIST(x)     hasType(x, ListType)

#define AS_SYM(val)    ((Symbol)AS_OBJ(val))
#define AS_LIST(val)   ((List)AS_OBJ(val))

#define SYM_HEAD(obj)   ((struct Symbol*)objStart((Object)(obj)))
#define SYM_BIND(obj)   (SYM_HEAD(obj)->bind)
#define SYM_BOUNDP(obj) (SYM_HEAD(obj)->isBound)
#define SYM_KEYWDP(obj) (SYM_HEAD(obj)->isKeyWord)

#define OBJ_TYPE(obj)  (objHead(obj)->type)
#define OBJ_HASH(obj)  (objHead(obj)->hash)
#define OBJ_FLAGS(obj) (objHead(obj)->flags)

#endif
