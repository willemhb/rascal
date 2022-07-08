#ifndef rascal_instructions_h
#define rascal_instructions_h

#include "common.h"
#include "rtypes.h"
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

  // op_mapping       = type_mapping,
  // op_array         = type_array,
  // op_function      = type_function,

  op_boolean       = type_boolean,
  op_character     = type_character,
  // op_builtin    = type_builtin,
  op_integer       = type_integer,
  op_fixnum        = type_fixnum,

  op_symbol        = type_symbol,
  op_cons          = type_cons,

  op_table         = type_table,
  op_string        = type_string,
  op_vector        = type_vector,

  op_error         = type_error,
  op_bytecode      = type_bytecode,
  op_closure       = type_closure,
  op_port          = type_port,

  /* other builtins */
  op_idp           = N_TYPES, // id?
  op_eqlp,                    // =?
  op_isap,                    // isa?

  op_ord,                     // ord
  op_sizeof,                  // size
  op_typeof,                  // type

  op_setv,                    // :=

  /* arithmetic builtins */
  op_add,                     // +
  op_sub,                     // -
  op_mul,                     // *
  op_div,                     // /
  op_mod,                     // mod
  op_eqp,                     // =
  op_ltp,                     // <

  /* accessors (mutators start with an x and are in-place unless a value is protected) */
  op_car,                     // car - car accessor
  op_cdr,                     // cdr - cdr accessor
  op_xar,                     // xar - car mutator
  op_xdr,                     // xdr - cdr mutator
  op_ref,                     // ref - key accessor
  op_xef,                     // xef - key mutator
  op_put,                     // put - returns collection that includes given key or value
  op_xut,                     // xut - add key or value to collection

  /* sequences & collections */
  op_len,                     // len
  op_emptyp,                  // empty?
  op_hasp,                    // has?
  op_fst,                     // fst
  op_rst,                     // rst
  op_join,                    // join   - add at front
  op_append,                  // append - add at back
  op_conj,                    // conj   - add in optimal manner

  /* low-level io */
  op_open,                    // open
  op_close,                   // close
  op_princ,                   // princ
  op_readc,                   // readc
  op_peekc,                   // peekc

  /* character and string handling */
  op_ctypep,                  // char-type?
  op_upper,                   // upper
  op_lower,                   // lower

  /* core */
  op_read,                    // read
  op_eval,                    // eval
  op_prin,                    // prin
  op_load,                    // load
  op_comp,                    // comp
  op_exec,                    // exec

  /* error runtime utilities */
  op_errorb,                  // error! - creates and raises error

  /* low-level environment interaction */
  op_lookup,                  // lookup
  op_boundp,                  // bound?
  op_extendb,                 // extend!
  op_bindb,                   // bind!
  op_assignb,                 // assign!
  op_captureb,                // capture!

  /* system interaction */
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
  op_load_locallocal,
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
extern ensure_t      Ensure[form_pad];
extern flags_t       BuiltinFlags[form_pad];

extern size_t        InstructionArgC[num_instructions];
extern char_t       *InstructionNames[num_instructions];

#endif
