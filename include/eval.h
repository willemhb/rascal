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

   // functions & closures ----------------------------------------------------
   op_capture, op_call, op_tail_call, op_return,

   num_instructions
  } opcode_t;

typedef ushort instruction_t;

// globals --------------------------------------------------------------------
// global variables -----------------------------------------------------------
extern table_t  *Namespace;
extern vector_t *Globals;

// stacks & registers ---------------------------------------------------------
extern vector_t *Stack;
extern vector_t *Values;
extern object_t *Fun;
extern value_t   Val;
extern uint      Prc;

// forward declarations -------------------------------------------------------
value_t exec(function_t *code);

#endif
