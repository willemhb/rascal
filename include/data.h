#ifndef rl_data_h
#define rl_data_h

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

typedef struct {
  ExpType type;
  char*   name;
  size_t  obsize;
} ExpTypeInfo;

extern ExpTypeInfo Types[];

#define HEAD                                    \
  Obj* heap;                                    \
  ExpType type;                                 \
  bool marked

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

#define OBJ     0xfffc000000000000ul
#define NUL     0xffff000000000000ul
#define EOS_T   0x7ffd000000000000ul
#define EOS     0x7ffd0000fffffffful

// forward declarations
// expression APIs
ExpType expr_type(Expr x);

// object API
void   free_object(void *obj);

// symbol API
Sym*   mk_sym(char* chars);

// list API
List*  mk_list(size_t n, Expr* xs);

// 

// convenience macros
#define as_sym(x)  ((Sym*)as_obj(x))
#define as_list(x) ((List*)as_obj(x))

#endif
