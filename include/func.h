#ifndef rl_func_h
#define rl_func_h

#include "obj.h"

#include "def/opcodes.h"

/* C types */
typedef enum func_fl_t func_fl_t;

struct func_head_t {
  char   *name;
  int   (*validate)(size_t n, val_t *args);
  int    nargs;
  bool   vargs;
  obj_t  obj;
};

enum func_fl_t {
  vargs_func=128
};

struct module_t {
  code_t bcode;
  vec_t  consts;
};

struct native_t {
  val_t (*funptr)(size_t nargs, val_t *args);
};

struct prim_t {
  opcode_t label;
};

/* API */
native_t make_native(char *name, int nargs, bool vargs, native_fn_t funptr);
val_t    native(char *name, int nargs, bool vargs, native_fn_t funptr);
val_t    def_native(char *name, int nargs, bool vargs, native_fn_t funptr);

prim_t make_prim(char *name, int nargs, bool vargs, opcode_t label);
val_t  prim(char *name, int nargs, bool vargs, opcode_t label);
val_t  def_prim(char *name, int nargs, bool vargs, opcode_t label);

module_t make_module(char *name, int nargs, bool vargs, code_t bcode, vec_t consts);
val_t    module(char *name, int nargs, bool vargs, code_t bcode, vec_t consts);
val_t    def_module(char *name, int nargs, bool vargs, code_t bcode, vec_t consts);
void     finalize_module(module_t module);
size_t   put_module_const(module_t module, val_t x);
val_t    get_module_const(module_t module, size_t n);
size_t   emit_instr(module_t module, ushort op, ...);

gen_decl_2(bool, is_native, val, obj);
gen_decl_2(bool, is_prim, val, obj);
gen_decl_2(bool, is_module, val, obj);
gen_decl_2(bool, is_func, val, obj);
gen_decl_3(native_t, as_native, val, obj, native);
gen_decl_3(prim_t, as_prim, val, obj, prim);
gen_decl_3(module_t, as_module, val, obj, module);
gen_decl_5(struct func_head_t*, func_head, val, obj, native, prim, module);

#define func_head(x)                            \
  _Generic((x),                                 \
           val_t:val_func_head,                 \
           obj_t:obj_func_head,                 \
           native_t:native_func_head,           \
           prim_t:prim_func_head,               \
           module_t:module_func_head)((x))

#define func_name(x)     (func_head(x)->name)
#define func_nargs(x)    (func_head(x)->nargs)
#define func_vargs(x)    (func_head(x)->vargs)
#define native_funptr(x) gen_getf(x, funptr, native)
#define prim_label(x)    gen_getf(x, label, prim)
#define module_bcode(x)  gen_getf(x, bcode, module)
#define module_consts(x) gen_getf(x, consts, module)

/* initialization */
void func_init(void);


#endif
