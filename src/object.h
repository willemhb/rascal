#ifndef object_h
#define object_h

#include "value.h"
#include "type.h"

// C types --------------------------------------------------------------------
#define HEADER					\
  Obj* next;					\
  uint  flags : 24;				\
  uint  black :  1;				\
  uint  gray  :  1;				\
  uint  type  :  6;				\
  uint  arity

typedef uint64 uhash;

typedef struct {
  Obj** array;
  uint  count, cap;
} Objs;

typedef enum {
  // common flags -------------------------------------------------------------
  NOFREE     =0x800000,
  FROZEN     =0x400000,
  INITIALIZED=0x200000,

  // symbol flags -------------------------------------------------------------
  GENERATED  =0x000001,
  LITERAL    =0x000002,

  // func flags ---------------------------------------------------------------
  NATIVE     =0x000001,
  USER       =0x000002,
  CLOSURE    =0x000004,
  TOPLEVEL   =0x000008,
  SCRIPT     =0x000010,
  VARGS      =0x000020,
  VOPTS      =0x000040,
  VKWARGS    =0x000080,

  // bin flags ----------------------------------------------------------------
  ENCODED    =0x000001,

  // table flags --------------------------------------------------------------
  EQUAL      =0x000001
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
void init_obj(void* self, Type type, flags fl);
void mark_obj(void* self);
void destruct_obj(void* self);

bool has_flag(void* self, flags fl);
bool set_flag(void* self, flags fl);
bool del_flag(void* self, flags fl);

// lifetimes ------------------------------------------------------------------
Sym*   get_sym(bool genp, char* name);
Sym*   mk_sym(char* name);
void   init_sym(Sym* self, flags fl, char* name);

Func*  new_func(flags fl, uint arity, Sym* name, Mtable* type, void* func);
Func*  mk_func(bool userp, void* func);
void   init_func(Func* self, flags fl , uint arity, Sym* name, Mtable* type, void* func);

List*  new_list(Val head, List* tail);
List*  mk_list(void);
void   init_list(List* self, flags fl, Val head, List* tail);

Bin*   new_bin(bool encp, uint n, void* data);
Bin*   mk_bin(void);
void   init_bin(Bin* self, flags fl, uint n, void* data);

Table* new_table(bool eqlp, uint n, Val* data);
Table* mk_table(void);
void   init_table(Table* self, flags fl, uint n, Val* data);

// accessors & mutators -------------------------------------------------------
Val    list_nth(List* list, uint n);
List*  list_assoc(List* list, Val k);

ubyte  bin_get(Bin* b, uint n);
ubyte  bin_set(Bin* b, uint n, ubyte xx);
uint   bin_write(Bin* b, uint n, void* data);
void   resize_bin(Bin* b, uint n);

Val   *table_nth(Table* table, uint n);
Val    table_get(Table* table, Val key);
Val    table_set(Table* table, Val key, Val val);
Val    table_del(Table* table, Val key);
void   resize_table(Table* table, uint n);

// objs api -------------------------------------------------------------------
void init_objs(Objs* objs);
void free_objs(Objs* objs);
void resize_objs(Objs* objs, uint n);
uint push_objs(Objs* objs, Obj* obj);
Obj* pop_objs(Objs* objs);

#endif
