#ifndef rascal_instructions_h
#define rascal_instructions_h

#include "common.h"
#include "types.h"

/*
  rascal uses 16-bit opcodes, 12 of which encode the operation and 2 of which encode
  information about the operator arguments.

  opcodes are divided into three ranges.

  The low range is reserved for constructors and builtins.

  The middle range is reserved for special forms.

  The high range is reserved for true opcodes, ie, opcodes that don't represent a
  rascal callable function.
*/

enum builtin_t {
  op_null          = type_null,
  op_none          = type_none,
  op_any           = type_any,

  op_fixnum        = type_fixnum,
  op_integer       = type_integer,

  op_boolean       = type_boolean,
  op_character     = type_character,

  op_cons          = type_cons,
  op_symbol        = type_symbol,

  op_vector        = type_vector,
  op_table         = type_table,
  op_dict          = type_dict,
  op_string        = type_string,
  op_bytecode      = type_bytecode,

  op_port          = type_closure,
  op_closure       = type_port,

  /* other builtins */
  /* utilities */
  op_idp  = TYPE_PAD, // id?
  op_eqlp,            // =?
  op_isap,            // isa?

  /* arithmetic */
  op_add,
  op_sub,
  op_mul,
  op_div,
  op_mod,
  op_eqp,
  op_ltp,

  /* accessors */
  op_len,
  op_ref, op_xef,
  op_put, op_xut,
  op_car, op_xar,
  op_cdr, op_xdr,

  /* core */
  op_read,
  op_eval,
  op_prin,
  op_load,
  op_exec,
  op_comp,
  op_apply,

  /* low-level io & character handling */
  op_open,
  op_close,
  op_princ,
  op_readc,
  op_peekc,
  op_ctypep,
  op_upper,
  op_lower,

  /* system interface */
  op_boundp,
  op_lookup,
  op_assignb,
  op_extendb,
  op_captureb,
  op_errorb,

  /* os interaction */
  op_exit,                    // exit - (calls C exit)
  op_system,                  // system - (calls C system)
  builtin_pad
};

enum form_t {
  op_quote = builtin_pad,
  op_do,
  op_if,
  op_lambda,
  op_macro,
  form_pad
};

enum opcode_t {
  /* no-op */
  op_noop = form_pad,

  /* stack manipulation */
  op_push,
  op_pop,

  /* loads */
  op_load_val,
  op_load_global,
  op_load_local,
  op_load_formal,
  op_load_closure,

  /* stores */
  op_store_global,
  op_store_local,
  op_store_formal,
  op_store_closure,

  /* control flow */
  op_jump,
  op_jump_true,
  op_jump_false,

  /* guards */
  op_argco,
  op_vargco,

  /* function calls & environment */
  op_call,
  op_return,
  op_Mk_closure,
  op_capture,

  /* misc */
  op_finished,

  num_instructions
};

union instruction_t {
  opcode_t  opcode;
  form_t    form;
  builtin_t builtin;
  int       bits;
};

// instructions dispatch tables
extern Cbuiltin_t    Builtins[form_pad];

extern size_t        InstructionArgC[num_instructions];
extern char         *InstructionNames[num_instructions];

#endif
