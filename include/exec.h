#ifndef rascal_exec_h
#define rascal_exec_h

#include "common.h"

// C types --------------------------------------------------------------------
typedef enum {
  HALT, NOOP,

  PUSH, POP, DUP,

  LOADNIL, LOADTRUE, LOADFALSE, LOAD0, LOAD1,

  LOADI16, LOADC16,

  LOADVAL, LOADGLO, STOREGLO, LOADLOC, STORELOC, LOADUPV, STOREUPV,

  JUMP, JUMPTRUE, JUMPFALSE,

  CALL, RETURN
} opcode_t;

// utilities ------------------------------------------------------------------
value_t rsp_exec(value_t code);

// native functions -----------------------------------------------------------
value_t native_exec(value_t *args, size_t n_args);

// initialization -------------------------------------------------------------
void exec_init(void);

#endif
