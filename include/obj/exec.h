#ifndef rascal_obj_exec_h
#define rascal_obj_exec_h

#include "obj/array.h"

typedef struct control_t   control_t;
typedef struct upvalues_t  upvalues_t;
typedef struct upvalue_t   upvalue_t;
typedef struct stack_t     stack_t;
typedef struct vm_t        vm_t;
typedef struct closure_t   closure_t;

struct control_t
{
  OBJECT;

  arity_t   base;     // offset of locals
  arity_t   size;     // number of locals

  opcode_t *prgc;     // program counter

  object_t *stack;    // value stack
  object_t *caller;   // calling frame
  object_t *prompt;   // bottom of current continuation chain 
  object_t *upvalues; // closed upvalues
  object_t *code;     // executing code object
};

struct closure_t
{
  OBJECT;

  object_t *bytecode; // bytecode object
  object_t *upvalues; // captured upvalues
};

struct stack_t
{
  ARRAY(value_t);
};

struct upvalues_t
{
  ARRAY(object_t*);
};

struct upvalue_t
{
  OBJECT;
  bool       closed;
  arity_t    location;
  value_t    value;
  upvalue_t *next;
};

// vm state type
#define CALLSTACK_CAP 2048
#define VALSTACK_CAP  (UINT16_MAX+1)

struct vm_t
{
  OBJECT;
  
  value_t    value;                 // transmit return values (val register)
  object_t  *control;               // currently executing frame
  object_t  *module;                // currently executing module
  object_t  *upvalues;              // open upvalues
  object_t  *stack;                 // vm stack

  control_t  framepool[CALLSTACK_CAP];
  value_t    valpool[VALSTACK_CAP];
};

// globals
extern vm_t Vm;
extern stack_t Stack;

extern type_t ControlType, UpvaluesType, UpvalueType, StackType, VmType, ClosureType;

#endif
