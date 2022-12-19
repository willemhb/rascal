#ifndef rascal_opcode_h
#define rascal_opcode_h

#include "common.h"

// virtual machine
typedef enum opcode_t
  {
    /* misc */
    op_invalid        = 0,
    op_halt           = 1, // (0) - exit the interpreter
    op_nothing        = 2, // (0) - do nothing

    /* stack manipulation */
    op_pop            = 3, // (0) - pop TOS
    op_dup            = 4, // (0) - duplicate TOS
 
    /* load/store instructions */
    /* special constants */
    op_load_nul       = 4,
    op_load_true      = 5,
    op_load_false     = 6,

    /* general load/store */
    op_load_const     = 7, /* load stored constant */
    op_load_local     = 8, /* load local variable (on stack) */
    op_store_local    = 9, /* store in local variable (on stack) */
    op_load_upvalue   =10, /* load shared non-local variable */
    op_store_upvalue  =11, /* store in shared non-local variable */
    op_load_package   =12, /* load package-level variable */
    op_store_package  =13, /* store in package-level variable */
    op_load_global    =14, /* load from toplevel (used in repl and for loading modules) */
    op_store_global   =15, /* store at toplevel */

    /* bind instructions */
    op_bind           =16, /* pop TOS, push to envt */
    op_bind_n         =17, /* pop the top argx values from stack and store them in envt, starting from BP */
    op_bind_v         =18, /* pop the top (argc - argx) values from stack into a new list, do as op_bind_n, bind result in envt */
    op_unbind_n       =19, /* pop argx values from envt */

    /* closures & upvalues */
    op_load_closure   =20, /* create a new closure for module at TOS. */
    op_capture_upvalue=21, /* get an open upvalue for the variable at offset [argx][argy], creating one if necessary. Add it to the upvalues of the module at TOS. */
    op_close_upvalue  =22, /* move the value for upvalue at control->upvalues[argx] off the stack. Remove the upvalue from the open list. */

    /* control flow */
    /* branching */
    op_jump_true      =23,
    op_jump_false     =24,
    op_jump           =25,

    /* function dispatch & return */
    op_invoke         =26,
    op_invoke_generic =27,
    op_invoke_prim    =28,
    op_invoke_native  =29,
    op_invoke_module  =30,
    op_invoke_effect  =31,
    op_return         =32,

    /* flags */
    op_set_panicp     =33,
    op_clear_panicp   =34,
    op_set_tailp      =35,
    op_clear_tailp    =36,

    op_set_toplevelp  =37,
    op_clear_toplevelp=38,
    op_set_modulep    =39,
    op_clear_modulep  =40,
    op_set_closurep   =41,
    op_clear_closurep =42,

    /* effect & prompt handling */
    op_enter_prompt   =43, /* create a new prompt (basically save the current frame, then reset pp to TOF) */
    op_leave_prompt   =44, /* exit the current prompt, preserving the top argx values on the stack. */
    op_capture_prompt =45, /* create a control frame that reifies the continuation starting from pp[argx]. Ignore the top argy values on the stack. */
    op_reenter_prompt =46, /* restore the prompt at TOS - argx */

    /* packages & imports */
    op_use_package    =47, /* Execute the package with global ID argx if it has not already been loaded (NB: interpreter called recursively). Push it to the stack. */
    op_use_binding    =48, /* Get the value at TOS->bindings[argx] and push it to control->bindings */
    op_use_methods    =49, /* Add all methods from the function at TOS->bindings[argx] to the method table for the function of the same name at control->bindings[argy] */

    /* multimethods */
    op_insert_method  =50, /* Add closure at TOS with signature at TOS-1 to function at TOS-2. Pop top 2 values (leaving function at TOS). */

    /* primitive labels */
    op_idp            =256,
    op_not            =258,
    op_typeof         =259,
  } opcode_t;

static inline size_t op_argc( opcode_t op )
{
  switch ( op )
    {
    case op_invalid ... op_load_false:
      return 0;

    case op_load_const ... op_store_global:
      return 1;

    case op_bind:
      return 0;

    case op_bind_n ... op_unbind_n:
      return 1;

    case op_load_closure:
      return 0;

    case op_capture_upvalue:
      return 2;

    case op_close_upvalue:
      return 1;

    case op_jump_true ... op_invoke_effect:
      return 1;

    case op_return ... op_enter_prompt:
      return 0;

    case op_leave_prompt ... op_use_methods:
      return 1;

    default:
      return 0;
    }
}

#endif
