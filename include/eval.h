#ifndef rascal_eval_h
#define rascal_eval_h

#include "function.h"
#include "list.h"

// C types --------------------------------------------------------------------
typedef enum
  {
   // exit point --------------------------------------------------------------
   op_halt=num_builtins,

   // stack manipulation ------------------------------------------------------
   op_push, op_pop, op_dup,

   // loads & stores ----------------------------------------------------------
   op_load_const,

   op_load_local,   op_store_local,

   op_load_closure, op_store_closure,

   op_load_global,  op_store_global,

   // control flow ------------------------------------------------------------
   op_jump, op_jump_true, op_jump_false,

   op_catch, op_throw,

   // functions & closures ----------------------------------------------------
   op_capture, op_call, op_return,

   num_instructions
  } opcode_t;

typedef ushort instruction_t;

typedef enum
  {
    exec_fl_catch   =16,
    exec_fl_captured=32
  } exec_fl_t;

// globals --------------------------------------------------------------------
// global variables -----------------------------------------------------------
extern table_t  *Namespace;
extern vector_t *Globals;

// stacks & registers ---------------------------------------------------------
extern vector_t *Stack, *Values;
extern object_t *Fn;
extern value_t   Val;
extern uint      Pc;

#define Bp    (Values->base.arity)
#define Fp    (Stack->base.arity)
#define Sp    (Stack->len)
#define Vp    (Values->len)

#define Envt  (Values->data+Bp)
#define Clo   (Envt[0])
#define Frame (Stack->data+Fp)

/**
 *
 *  stack layout
 *
 *  +----+----+----+----+
 *  | Fn | Pc | Bp | Fp |
 *  +----+----+----+----+
 *                      ^
 *                      |
 *                      Fp
 *
 *  enviroment layout
 *
 *  +-----+-----+-----+-----+
 *  | Clo | v-1 | ... | v-n |
 *  +-----+-----+-----+-----+
 * 
 **/

// forward declarations -------------------------------------------------------
value_t exec(function_t *code);

// macros & statics -----------------------------------------------------------
static inline im_flag_p(catch, CHARACTER, exec_fl_catch)
static inline im_flag_p(captured, CHARACTER, exec_fl_captured)

#endif
