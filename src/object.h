#ifndef object_h
#define object_h

#include <stdio.h>

#include "value.h"

/* C types */
// flags ----------------------------------------------------------------------
typedef enum ObjType ObjType;
typedef enum ObjFl ObjFl;

typedef union Native Native;
typedef void (*GuardFn)(Obj *callee, int n_args, Val *args);

// user objects ---------------------------------------------------------------
typedef struct Sym Sym;
typedef struct Pair Pair;
typedef struct List List;
typedef struct Vec Vec;
typedef struct Bin8 Bin8;
typedef struct Bin16 Bin16;
typedef struct Str Str;
typedef struct Table Table;
typedef struct Port Port;
typedef struct NativeFn NativeFn;
typedef struct UserFn UserFn;

enum ObjType {
  // user objects
  SYM_OBJ=OBJ_VAL,
  PORT_OBJ,
  NATIVE_FN_OBJ,
  USER_FN_OBJ,
  PAIR_OBJ,
  LIST_OBJ,
  VEC_OBJ,
  TABLE_OBJ,
  BIN8_OBJ,
  BIN16_OBJ,
  STR_OBJ,
};

#define NUM_OBJ_TYPES (STR_OBJ+1)

struct Obj {
  Obj *next;
  ObjType type;
  uint flags : 29;
  uint is_static : 1;
  uint black : 1;
  uint gray  : 1;
};

// symbol flags
enum {
  BOUND_SYM=1,     // has toplevel binding
  LITERAL_SYM=2,   // self-evaluating
  INTERNED_SYM=4,  // interned in symbol table
  CONSTANT_SYM=8   // cannot be rebound at top level
};

struct Sym {
  Obj obj;
  char *name;
  uint64 hash, idno;
  Val bind; // toplevel binding
  Sym *left, *right; // invasive symbol table
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

struct Bin8 {
  Obj obj;
  uint8* array;
  int count, cap;
};

struct Bin16 {
  Obj obj;
  uint16* array;
  int count, cap;
};

struct Str {
  Obj obj;
  char *array;
  int count, cap;
  uint64 hash;
};

struct Table {
  Obj obj;
  Pair **table;
  int count, cap;
  union {
    sint8 *ord8;
    sint16 *ord16;
    sint32 *ord32;
  };
  Obj *compare; // optional, not yet used
};

struct Port {
  Obj obj;
  FILE *ios;
};

union Native {
  // common
  Val (*thunk)(void);
  Val (*unary)(Val x);
  Val (*binary)(Val x, Val y);
  Val (*ternary)(Val x, Val y, Val z);
  Val (*nary)(int n_args, Val *args);

  // special
  Val (*reader)(Glyph x, FILE *ios);
};

// function flags
enum {
  VARGS_FUN=1,
  READER_FUN=2
};

struct NativeFn {
  Obj obj;
  Sym *name;
  int argc;
  bool vargs;
  GuardFn guard;
  Native func;
};

struct UserFn {
  Obj  obj;
  Str *name;
  int n_args;
  bool vargs;
  List *ns;
  List *env;       // null if not a closure
  Vec *consts;
  Bin16 *code;
};

/* globals */
#define CAP_MIN        8
#define TABLE_PRESSURE 0.625

extern List EmptyList;
extern Sym *SymbolTable;

/* API */
// common utilities
ObjType obj_type(Val val);
usize obj_size(Obj *obj);
char *obj_type_name(Obj *obj);
bool is_obj_type(Val value, ObjType type);

// casts
Sym* as_sym(Val x);
Pair* as_pair(Val x);
List* as_list(Val x);
Vec* as_vec(Val x);
Table* as_table(Val x);
Bin8* as_bin8(Val x);
Bin16* as_bin16(Val x); 
Str* as_str(Val x);
Port* as_port(Val x);
NativeFn* as_native_fn(Val x);
UserFn* as_user_fn(Val x);

// constructors ---------------------------------------------------------------
Obj* new_obj(ObjType type);
Obj* new_objs(ObjType type, int n);

UserFn *mk_user_fn(char *name, int n_args, bool vargs);
Table *mk_table(void);
void init_table(Table *table, bool is_static);
Str *mk_str(int n_chars, char *chars);

Sym *new_sym(char *name);
Pair *new_pair(Val fst, Val snd);
List *new_list(int n_args, Val *args);
Vec *new_vec(int n_args, Val* args);
Table *new_table(int n_args, Val* args);
Bin8* new_bin8(int n_args, Val* args);
Bin16* new_bin16(int n_args, Val* args);
Str *new_str(int n_args, Val *args);
Port *new_port(FILE *ios);
NativeFn *new_native_fn(char *name, int n_args, bool vargs, GuardFn guard, Native func);
UserFn *new_user_fn(char *name, int n_args, bool vargs, List *ns, List *env, Vec *consts, Bin16 *code);

Val sym(char *name);
Val list(int n_args, Val *args);
Val pair(Val fst, Val snd);
Val vec(int n_args, Val *args);
Val str(char *chars);
Val bin8(int n_args, ubyte* bytes);
Val bin16(int n_args, uint16* );
Val table(int n_args, Val *args);
Val port(FILE *ios);

// destructors ----------------------------------------------------------------
void free_obj(Obj *obj);
void free_table(Table *table);

// accessors ------------------------------------------------------------------
Val pair_fst(Pair *pair);
Val pair_snd(Pair *pair);
Val pair_fxt(Pair *pair, Val fst);
Val pair_sxd(Pair *pair, Val snd);

Val list_head(List *list);
List *list_tail(List *list);
List *list_find(List *list, Val x);
List *list_assoc(List *list, Val k);
Val list_nth(List *list, int n);

Val *vec_peep(Vec *vec, int i);
Val *vec_find(Vec *vec, Val x);
Val vec_ref(Vec* vec, int i);
Val vec_set(Vec* vec, int i, Val x);
int vec_add(Vec* vec, Val x);
int vec_del(Vec* vec, int i, Val* buf);
int vec_addn(Vec* vec, int n, Val* xs);

usize bin8_len(Bin8* bin);
ubyte *bin8_find(Bin8* bin, ubyte x);
ubyte *bin8_peep(Bin8* bin, int i);
ubyte bin8_ref(Bin8* bin, int i);
ubyte bin8_set(Bin8* bin, int i, ubyte x);
int bin8_add(Bin8* bin, ubyte x);
int bin8_del(Bin8* bin, int i, ubyte* buf);
int bin8_addn(Bin8* bin, int n, ubyte* xs);

usize str_len(Str* str);
char *str_peep(Str* str, int i);
char *str_find(Str* str, Glyph x);
Glyph str_ref(Str* str, int i);

usize table_cnt(Table* table);
Pair **table_find(Table* table, Val key);
bool  table_ref(Table* table, Val key, Val* buf);
bool  table_set(Table* table, Val key, Val val);
bool  table_add(Table* table, Val key, Val val);
bool  table_del(Table* table, Val key, Pair** buf);

// predicates -----------------------------------------------------------------
// type predicates ------------------------------------------------------------
bool is_sym(Val x);
bool is_pair(Val x);
bool is_list(Val x);
bool is_vec(Val x);
bool is_bin8(Val x);
bool is_str(Val x);
bool is_table(Val x);
bool is_port(Val x);
bool is_user_fn(Val x);
bool is_native_fn(Val x);

// value predicates -----------------------------------------------------------
bool is_empty_list(Val x);
bool is_nonempty_list(Val x);

bool is_literal_sym(Val x);
bool is_variable_sym(Val x);
bool is_interned_sym(Val x);
bool is_bound_sym(Val x);
bool is_const_sym(Val x);

bool is_vargs_fn(Obj *obj);

// misc utilities -------------------------------------------------------------
Val lookup(Val x);

#endif
