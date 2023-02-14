#ifndef object_h
#define object_h

#include <stdio.h>

#include "value.h"

/* C types */
typedef enum  ObjType  ObjType;
typedef enum  ObjFl    ObjFl;
typedef enum  SymFl    SymFl;
typedef enum  NativeFl NativeFl;

typedef struct Sym     Sym;
typedef struct List    List;
typedef struct Pair    Pair;
typedef struct Vec     Vec;
typedef struct Table   Table;
typedef struct Bin     Bin;
typedef struct Port    Port;

typedef struct Closure Closure;
typedef struct Chunk   Chunk;
typedef struct Native  Native;
typedef struct UpVal   UpVal;

enum ObjType {
  /* user objects */
  SYM_OBJ,
  LIST_OBJ,
  PAIR_OBJ,
  VEC_OBJ,
  TABLE_OBJ,
  BIN_OBJ,
  PORT_OBJ,

  /* internal objects */
  CLOSURE_OBJ,
  CHUNK_OBJ,
  NATIVE_OBJ,
  UPVAL_OBJ
};

#define NUM_OBJ_TYPES (UPVAL_OBJ+1)

enum ObjFl {
  STATIC_OBJ=0x8000
};

struct Obj {
  Obj     *next;
  ObjType  type;
  ushort   flags;
  ubyte    black;
  ubyte    gray;
};

enum SymFl {
  INTERNED_SYM=0x0001,
  LITERAL_SYM =0x0002,
  BOUND_SYM   =0x0004
};

struct Sym {
  Obj     obj;
  char   *name;
  uint64  hash;
  uint64  idno;
  Val     bind; // toplevel binding

  // invasive tree of symbols (for interned symbols)
  Sym    *left, *right;
};

struct List {
  Obj    obj;
  List  *tail;
  usize  arity;
  Val    head;
};

struct Pair {
  Obj    obj;
  Val    cdr;
  uint64 hash;
  Val    car;
};

struct Vec {
  Obj    obj;
  Val   *array;
  usize  count;
  usize  cap;
};

struct Table {
  Obj    obj;
  Pair **table;
  usize  count;
  usize  cap;
};

struct Bin {
  Obj    obj;

  union {
    uint8  *u8;
    uint16 *u16;
    uint32 *u32;
    void   *array;
  };

  usize  count;
  usize  cap;
};

struct Port {
  Obj   obj;
  FILE *ios;
};

struct Closure {
  Obj    obj;
  Chunk *chunk;
  Vec   *upvals;
};

struct Chunk {
  Obj    obj;
  int    arity;
  int    n_upvals;
  Vec   *consts;
  Bin   *code;
};

enum NativeFl {
  NATIVE_THUNK,
  NATIVE_UFUNC,
  NATIVE_BFUNC,
  NATIVE_TFUNC,
  NATIVE_RFUNC
};

struct Native {
  Obj     obj;
  funcptr func;
};

struct UpVal {
  Obj    obj;
  Val   *location;
  Val    closed;
  UpVal *next;
};

/* globals */
#define CAP_MIN        8
#define TABLE_PRESSURE 0.625

extern Sym *SymbolTable;

/* convenience macros */
#define OBJ_TYPE(value)   (AS_OBJ(value)->type)
#define IS_SYM(value)     is_obj_type(value, SYM_OBJ)
#define IS_LIST(value)    is_obj_type(value, LIST_OBJ)
#define IS_PAIR(value)    is_obj_type(value, PAIR_OBJ)
#define IS_VEC(value)     is_obj_type(value, VEC_OBJ)
#define IS_TABLE(value)   is_obj_type(value, TABLE_OBJ)
#define IS_BIN(value)     is_obj_type(value, BIN_OBJ)
#define IS_PORT(value)    is_obj_type(value, PORT_OBJ)
#define IS_CLOSURE(value) is_obj_type(value, CLOSURE_OBJ)
#define IS_CHUNK(value)   is_obj_type(value, CHUNK_OBJ)
#define IS_NATIVE(value)  is_obj_type(value, NATIVE_OBJ)
#define IS_UPVAL(value)   is_obj_type(value, UPVAL_OBJ)

#define AS_SYM(value)     ((Sym*)AS_OBJ(value))
#define AS_LIST(value)    ((List*)AS_OBJ(value))
#define AS_PAIR(value)    ((Pair*)AS_OBJ(value))
#define AS_VEC(value)     ((Vec*)AS_OBJ(value))
#define AS_TABLE(value)   ((Table*)AS_OBJ(value))
#define AS_BIN(value)     ((Bin*)AS_OBJ(value))
#define AS_PORT(value)    ((Port*)AS_OBJ(value))
#define AS_CLOSURE(value) ((Closure*)AS_OBJ(value))
#define AS_CHUNK(value)   ((Chunk*)AS_OBJ(value))
#define AS_NATIVE(value)  ((Native*)AS_OBJ(value))
#define AS_UPVAL(value)   ((UpVal*)AS_OBJ(value))

/* API */
/* common utilities */
usize   obj_size(Obj *obj);
ObjType obj_type(Val val);
bool    is_obj_type(Val value, ObjType type);
bool    is_static_obj(Obj *obj);

/* constructors */
Sym     *new_sym(char *name);
Val      sym(char *name);
List    *new_list(int n_args, Val *args);
Val      list(int n_args, Val *args);
Pair    *new_pair(Val car, Val cdr);
Vec     *new_vec(int n_args, Val *args);
Val      vec(int n_args, Val *args);
Table   *new_table(void);
Bin     *new_bin(int n_args, int el_size, void *data);
Port    *new_port(FILE *ios);
Closure *new_closure(Chunk *chunk);
Chunk   *new_chunk(void);
Native  *new_native(funcptr fn, NativeFl native);
UpVal   *new_upval(Val *slot);

/* accessors & mutator */
void  vec_clear(Vec *vec);
Val   vec_ref(Vec *vec, int i);
Val   vec_set(Vec *vec, int i, Val x);
usize vec_push(Vec *vec, Val x);
Val   vec_pop(Vec *vec);

void  bin_clear(Bin *bin);
uint  bin_ref(Bin *bin, int i);
uint  bin_set(Bin *bin, int i, uint u);
usize bin_push(Bin *bin, uint u);
usize buf_push(Bin *bin, uint u);
uint  bin_pop(Bin *bin);
void  bin_write(Bin *bin, usize n, void *data);
void  buf_write(Bin *bin, usize n, void *data);

void table_clear(Table *table);
bool table_ref(Table *table, Val key, Val   *buf);
bool table_set(Table *table, Val key, Val    val);
bool table_pop(Table *table, Val key, Pair **buf);

/* miscellaneous utilities */
void pad_vec(Vec *vec, usize count);
void pad_bin(Bin *bin, usize count);
void pad_buf(Bin *bin, usize count);
void pad_table(Table *table, usize count);

usize bin_elsize(Bin *bin);

#endif
