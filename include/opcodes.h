#ifndef rascal_opcodes_h
#define rascal_opcodes_h

/*
  rascal uses 16-bit opcodes, 10 of which encode the operation and 6 of which encode
  information about the operator arguments.
*/
typedef enum {
  /* no-op */
  op_none = 0x0000,

  /* stack manipulation */
  op_push          = 0x0001,
  op_pop           = 0x0002,

  /* loads */
  op_load_val      = 0x0003,
  op_load_global   = 0x0004,
  op_load_local    = 0x0005,
  op_load_formal   = 0x0006,
  op_load_closure  = 0x0007,

  /* stores */
  op_store_global  = 0x0008,
  op_store_local   = 0x0009,
  op_store_formal  = 0x000a,
  op_store_closure = 0x000b,

  /* control flow */
  op_jump          = 0x000c,
  op_jump_true     = 0x000d,
  op_jump_false    = 0x000e,

  /* guards */
  op_argco         = 0x000f,
  op_vargco        = 0x0010,

  /* function calls & environment */
  op_call          = 0x0011,
  op_return        = 0x0012,
  op_capture       = 0x0013,

  /* misc */
  op_finished      = 0x0013,
} opcode_t;

#endif
