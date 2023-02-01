#ifndef rascal_object_h
#define rascal_object_h

#include "value.h"

/* C types */
enum ObjType {
  SymbolObj=ObjectVal,
  ListObj,
  PairObj,
  TupleObj,
  MapObj,
  StringObj,
  BinaryObj,
  PortObj,
  StructObj,
  RecordObj,
  MapNodeObj,
  ChunkObj,
  ClosureObj,
  NativeObj,
  SpecialFormObj,
  UpValueObj
};

struct Object {
  Object  *next;
  usize    size;
  ObjType  type;
  uint32   flags  : 29;
  uint32   allocp :  1;
  uint32   black  :  1;
  uint32   gray   :  1;
};

struct Symbol {
  Object  obj;
  char   *name;
  uint64  idno; 
  Value   bind;
};

struct List {
  Object obj;
  List  *tail;
  usize  length;
  Value  head;
};

struct Pair {
  Object obj;
  Value  cdr;
  uhash  hash;
  Value  car;
};

struct Tuple {
  Object obj;
  Value *vals;
  usize  length;
};

struct Map {
  Object  obj;
  usize   length;
};

struct Chunk {
  Object  obj;
  uint16 *code;
  Tuple  *consts;
};

struct Closure {
  Object   obj;
  Chunk   *chunk;
  Objects *upvalues;
};

struct Native {
  Object   obj;

  RlError (*exec)(struct Interpreter *state, int nArgs);
};

struct SpecialForm {
  Object   obj;

  RlError (*comp)(struct Compiler *state, Value form);
};

struct UpValue {
  Object   obj;
  UpValue *next;
  bool     closed;
  int      index;
  Value    bind;
};

#include "decl/alist.h"
ALIST(Objects, Object*);

/* API */
ObjType obj_type(Object *obj);
bool    has_obj_type(Object *obj, ObjType type);
void    init_obj(Object *self, usize size, ObjType type);
void    free_obj(Object *self);

Symbol *symbol(char *name);
List   *list(Value *args, int nArgs);
Pair   *pair(Value car, Value cdr);
Tuple  *tuple(Value *args, int nArgs);

#endif
