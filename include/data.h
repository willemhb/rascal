#ifndef rl_data_h
#define rl_data_h

#include <stdio.h>

#include "common.h"
#include "collection.h"

// special indirecting containers for closure variables
// special values

// forward declarations

// list API
List*  as_list_s(char* f, Expr x);
List*  empty_list(void);
List*  mk_list(size_t n, Expr* xs);
List*  cons(Expr hd, List* tl);
Expr   list_ref(List* xs, int n);

// number API
Num       as_num_s(char* f, Expr x);
Num       as_num(Expr x);
Expr      tag_num(Num n);
uintptr_t as_fix(Expr x);
Expr      tag_fix(uintptr_t i);
void*     as_ptr(Expr x);
Expr      tag_ptr(void* ptr);

// convenience macros
#define as_list(x)     ((List*)as_obj(x))

#define is_fun(x)         has_type(x, EXP_FUN)
#define is_list(x)        has_type(x, EXP_LIST)

#endif
