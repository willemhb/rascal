#ifndef rl_data_h
#define rl_data_h

#include <stdio.h>

#include "common.h"

// Types
// Expression type codes
typedef enum {
    EXP_NUL=1,
    EXP_EOS,
    EXP_SYM,
    EXP_LIST,
    EXP_NUM
} ExpType;

// Expression types
typedef uintptr_t Expr;
typedef nullptr_t Nul;
typedef double    Num;
typedef struct    Obj  Obj;
typedef struct    Sym  Sym;
typedef struct    List List;

typedef union {
  Expr expr;
  Num  num;
  Obj* obj;
  Nul  nul;
} Val;

// Internal types
typedef void (*PrintFn)(FILE* ios, Expr x);
typedef void (*TraceFn)(void* ob);
typedef void (*FreeFn)(void* ob);

typedef struct {
  ExpType type;
  char*   name;
  size_t  obsize;
  PrintFn print_fn;
  TraceFn trace_fn;
  FreeFn  free_fn;
} ExpTypeInfo;

extern ExpTypeInfo Types[];

#define HEAD                                    \
  Obj* heap;                                    \
  ExpType type;                                 \
  byte black;                                   \
  byte gray;                                    \
  short flags

struct Obj {
  HEAD;
};

struct Sym {
  HEAD;

  char* val;
};

struct List {
  HEAD;

  Expr   head;
  List*  tail;
  size_t count;
};

// Expression tags
#define QNAN  0x7ffc000000000000ul

#define XTMSK 0xffff000000000000ul
#define XVMSK 0x0000fffffffffffful

#define OBJ    0xfffc000000000000ul
#define NUL    0xffff000000000000ul
#define EOS_T  0x7ffd000000000000ul
#define EOS    0x7ffd0000fffffffful

// forward declarations
// expression APIs
ExpType exp_type(Expr x);
ExpTypeInfo* exp_info(Expr x);

// object API
void* as_obj(Expr x);
Expr  tag_obj(void* obj);
void* mk_obj(ExpType type);
void  free_obj(void *obj);

// symbol API
Sym* mk_sym(char* chars);
bool sym_val_eql(Sym* s, char* v);

// list API
List*  empty_list(void);
List*  mk_list(size_t n, Expr* xs);
List*  cons(Expr hd, List* tl);

// number API
Num  as_num(Expr x);
Expr tag_num(Num n);

// convenience macros
#define head(x)    ((Obj*)as_obj(x))
#define as_sym(x)  ((Sym*)as_obj(x))
#define as_list(x) ((List*)as_obj(x))

#endif
