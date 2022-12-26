#ifndef rl_code_h
#define rl_code_h

#include "arr.h"

/* C types */
typedef struct code_head_t *code_head_t;

struct code_head_t {
  size_t len, cap;
  struct obj_head_t obj;
};

/* globals */
extern struct type_t CodeType;

/* API */
code_t make_code(size_t n, ushort *instr);
code_t code_from_obj(size_t n, val_t *instr);
val_t  code_ref(code_t code, int n);
val_t  code_set(code_t code, int n, val_t x);
size_t code_write(code_t *code, ushort op, ...);

/* convenience */
void dis_code(code_t code);

#define is_code(x)   has_type(x, &CodeType)
#define as_code(x)   ((code_t)as_obj(x))
#define code_head(x) ((code_head_t)obj_start((obj_t)(x)))

#endif
