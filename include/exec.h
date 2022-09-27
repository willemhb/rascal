#ifndef rascal_exec_h
#define rascal_exec_h

#include "obj.h"
#include "obj/envt.h"

typedef struct cons_t  cons_t;
typedef struct clo_t   clo_t;
typedef struct code_t  code_t;

typedef struct vm_t
{
  vals_t    stack;

  // vm state ('registers')
  clo_t    *exec;
  opcode_t *ip;
  
  arity_t   bp;
  arity_t   cnt;

  // upvalues list
  cons_t   *open_upvals;
} vm_t;

// globals --------------------------------------------------------------------
extern vm_t Vm;

// forward declarations
val_t exec( func_t *func );

// toplevel dispatch ----------------------------------------------------------
void exec_mark( void );
void exec_init( void );

#endif
