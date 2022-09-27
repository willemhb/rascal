#ifndef rascal_comp_h
#define rascal_comp_h

#include "val.h"

// C types --------------------------------------------------------------------
typedef enum
  {
   OP_POP,         OP_PUSH,
   OP_DUP,

   OP_LOAD_CONST,
   OP_LOAD_FAST,   OP_STORE_FAST,
   OP_LOAD_UPVAL,  OP_STORE_UPVAL,
   OP_LOAD_GLOBAL, OP_STORE_GLOBAL,

   OP_JUMP_TRUE,   OP_JUMP_FALSE,
   OP_JUMP,

   OP_CLOSURE,     OP_CLOSE_UPVAL,

   OP_CALL,        OP_DISPATCH,

   // capture a continuation
   OP_CNTL,

   OP_RETURN,      OP_HALT,
  } op_t;

// forward declarations -------------------------------------------------------
obj_t *comp_val(val_t val);
obj_t *comp_obj(obj_t *obj);
obj_t *comp_file(obj_t *fname);

// toplevel dispatch ----------------------------------------------------------
void comp_mark( void );
void comp_init( void );

#endif
