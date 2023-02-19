#ifndef object_h
#define object_h

#include "value.h"
#include "type.h"

// C types --------------------------------------------------------------------
#define HEADER					\
  Obj* next;					\
  uchar type;					\
  uchar nofree : 4;				\
  uchar black  : 2;				\
  uchar gray   : 2;				\
  uchar flags;					\
  uint arity

typedef uint64 uhash;

typedef struct {
  Obj* array;
  uint count, cap;
} Objs;

typedef enum {
  // symbol flags -------------------------------------------------------------
  LITERAL  =0x01,
  GENERATED=0x02,

  // func flags ---------------------------------------------------------------
  NATIVE   =0x01,
  USER     =0x02,
  CLOSURE  =0x04,
  VARGS    =0x08,
  VOPTS    =0x10,
  VKWARGS  =0x20,

  // bin flags ----------------------------------------------------------------
  ENCODED  =0x01,

  // table flags --------------------------------------------------------------
  EQUAL    =0x01
} ObjFl;

struct Obj {
  HEADER;
  ubyte space[0];
};

struct Sym {
  HEADER;
  char *name;
  uhash hash;
  Val   constant;
  Sym*  left, *right;
};

typedef struct {
  Table* vals;
  Bin*   code;
  List*  envt;
} Chunk;

struct Func {
  HEADER;
  Sym*    name;
  Mtable* mtable;
  void*   func;
};

struct List {
  HEADER;
  Val   head;
  List *tail;
};

struct Bin {
  HEADER;
  void *array;
  uint  count, cap;
};

typedef struct {
  Val key;
  Val val;
} Entry;

struct Table {
  HEADER;
  Entry *table;
  uint   count, cap;
  int   *ord;
};

// globals --------------------------------------------------------------------
extern Sym*  SymbolTable;
extern List  EmptyList;
extern Bin   EmptyString;

// API -------------------------------------------------------------------------
// common api ------------------------------------------------------------------
void init_obj(Obj* self, Type type, bool nofree, flags fl);
void mark_obj(Obj* self);
void destruct_obj(Obj* self);

// constructors ---------------------------------------------------------------
Sym*   get_sym(bool internp, char* name);
Func*  new_func(flags fl, uint arity, Val name, Mtable* type, void* func);
List*  new_list(Val head, List* tail);
Bin*   new_bin(bool encodedp, uint n, void* data);
Table* new_table(bool equalp, uint n, Val* data);

// objs api -------------------------------------------------------------------
void init_objs(Objs* objs);
void free_objs(Objs* objs);
void resize_objs(Objs* objs);
uint push_objs(Objs* objs, Obj* obj);
Obj* pop_objs(Objs* objs);

#endif
