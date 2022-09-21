#ifndef rascal_exec_h
#define rascal_exec_h

#include "obj/obj.h"

// C types --------------------------------------------------------------------
typedef uint16_t instr_t;

typedef struct vm_t
{
  OBJ_HEAD;
  obj_t    *stack;

  // vm state ('registers')
  obj_t    *code;
  obj_t    *envt;
  instr_t  *ip;
  arity_t   bp;
  arity_t   count;

  // upvalues list
  obj_t    *open_upvals;
} vm_t;

// globals --------------------------------------------------------------------
extern vm_t Vm;

// forward declarations & generics --------------------------------------------

// utilities ------------------------------------------------------------------


// toplevel dispatch ----------------------------------------------------------
void exec_mark( void );
void exec_init( void );


#endif
