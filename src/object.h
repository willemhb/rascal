#ifndef object_h
#define object_h

#include <stdio.h>

#include "value.h"

/* C types */
// flags ----------------------------------------------------------------------
typedef enum ObjType ObjType;
typedef enum ObjFl ObjFl;

// user objects ---------------------------------------------------------------
typedef struct Sym Sym;
typedef struct Pair Pair;
typedef struct List List;
typedef struct Vec Vec;
typedef struct Bin Bin;
typedef struct Str Str;
typedef struct Table Table;
typedef struct Port Port;
typedef struct NativeFn NativeFn;
typedef struct UserFn UserFn;
typedef struct ReaderFn ReaderFn;

enum ObjType {
  // user objects
  SYM_OBJ,
  PAIR_OBJ,
  LIST_OBJ,
  VEC_OBJ,
  TABLE_OBJ,
  BIN_OBJ,
  STR_OBJ,
  PORT_OBJ,
  NATIVE_FN_OBJ,
  USER_FN_OBJ,
  READER_FN_OBJ
};

#define NUM_OBJ_TYPES (READER_FN_OBJ+1)

struct Obj {
  Obj *next;
  ObjType type;
  uint flags : 29;
  uint is_static : 1;
  uint black : 1;
  uint gray  : 1;
};

struct Sym {
  Obj obj;
  char *name;
  uint64 hash;
  uint64 idno;
  Val bind; // toplevel binding
  bool interned, bound;
  // invasive tree of symbols (for interned symbols)
  Sym    *left, *right;
};

struct List {
  Obj obj;
  Val head;
  List *tail;
  usize length;
};

struct Pair {
  Obj obj;
  Val fst;
  Val snd;
  uint64 hash;
};

struct Vec {
  Obj obj;
  Val *array;
  int count, cap;
};

struct Bin {
  Obj obj;
  void *array;
  int count, cap;
};

struct Str {
  Obj obj;
  char *chars;
  int count, cap;
  uint64 hash;
};

struct Table {
  Obj obj;
  Pair **table;
  int count, cap;
};

struct Port {
  Obj obj;
  FILE *ios;
};

struct NativeFn {
  Obj obj;
  Str *name;
  int n_args;
  bool vargs;
  void (*guard)(NativeFn *native_fn, int n_args, Val *args);
  Val (*func)(int n_args, Val *args);
};

struct UserFn {
  Obj  obj;
  Str *name;
  int n_args;
  bool vargs;
  List *ns;
  List *env;       // null if not a closure
  Vec *consts;
  Bin *code;
};

// internal objects -----------------------------------------------------------
struct ReaderFn {
  Obj obj;
  Val (*func)(Glyph g, FILE *ios);
};

/* globals */
#define CAP_MIN        8
#define TABLE_PRESSURE 0.625

extern List EmptyList;
extern Sym *SymbolTable;

/* API */
/* common utilities */
ObjType obj_type(Val val);
usize obj_size(Obj *obj);
char *obj_type_name(Obj *obj);
bool is_obj_type(Val value, ObjType type);

// object type predicates
bool is_sym(Val x);
bool is_pair(Val x);
bool is_list(Val x);
bool is_vec(Val x);
bool is_bin(Val x);
bool is_str(Val x);
bool is_table(Val x);
bool is_port(Val x);
bool is_user_fn(Val x);
bool is_native_fn(Val x);
bool is_reader_fn(Val x);

// casts
Sym *as_sym(Val x);
Pair *as_pair(Val x);
List *as_list(Val x);
Vec *as_vec(Val x);
Table *as_table(Val x);
Bin *as_bin(Val x);
Str *as_str(Val x);
Port *as_port(Val x);
NativeFn *as_native_fn(Val x);
UserFn *as_user_fn(Val x);
ReaderFn *as_reader_fn(Val x);

// constructors
Obj *new_obj(ObjType type);
Obj *new_objs(ObjType type, int n);

UserFn *mk_user_fn(char *name, int n_args, bool vargs);

Sym *new_sym(char *name);
Pair *new_pair(Val fst, Val snd);
List *new_list(int n_args, Val *args);
Vec *new_vec(int n_args, Val *args);
Table *new_table(int n_args, Val *args);
Bin *new_bin(int n_bytes, ubyte *bytes);
Str *new_str(int n_chars, char *chars);
Port *new_port(FILE *ios);
NativeFn *new_native_fn(char *name, int n_args, bool vargs, void (*guard)(NativeFn *native, int n_args, bool vargs), Val (*func)(int n_args, Val *args));
UserFn *new_user_fn(char *name, int n_args, bool vargs, List *ns, List *env, Vec *consts, Bin *code);
ReaderFn *new_reader_fn(ReaderFn reader_fn);

#endif
