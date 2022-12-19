#ifndef rl_vm_obj_vm_h
#define rl_vm_obj_vm_h

#include "rascal.h"

/* commentary */

/* C types */
/* VM model

   10 registers in total, 3 stacks, 4 "pointers", flags, argc, and the currently executing module.

   The stacks:

   - stack - stores arguments and calling functions while arguments ae evaluated
   - envt  - stores local variables
   - frame - stores suspended calls

   The pointers:
   - bp    - the index in envt of the first local variable
   - ip    - instruction pointer
   - fp    - index in frame of the caller's frame
   - pp    - prompt pointer, ie, the location of the current enclosing continuation prompt

   The flags:
   - panicp    - indicate that the virtual machine is currently panicking
   - tailp     - indicate that current frame can be safely reused
   - globalp   - indicate that execution is at global scope (repl or imported package)
   - packagep  - indicate that execution is at package scope
   - funcp     - indicate that execution is at function scope
*/

typedef enum vm_fl_t vm_fl_t;

enum vm_fl_t {
  vm_fl_panicp    =  1,
  vm_fl_tailp     =  2,
  vm_fl_toplevelp =  4,
  vm_fl_modulep   =  8,
  vm_fl_closurep  = 12
};

typedef struct vm_t
{
  /* main registers */
  stack_t   *stack;          // arguments to the currently executing function
  stack_t   *envt;           // local variables
  stack_t   *frame;          // call frames
  closure_t *control;        // currently executing function or module

  uint      bp, ip, fp, pp, fl, argc;

  /* ephemeral registers (not saved) */
  val_t     val;

  /* other global state */
  ns_t     *toplevel;        // toplevel namespace (stores variables created in REPL and loaded modules).
  upval_t  *open;            // list of open upvalues
} vm_t;

/* globals */
extern vm_t Vm;

/* API */

/* runtime */
void rl_vm_obj_vm_init( void );
void rl_vm_obj_vm_mark( void );
void rl_vm_obj_vm_cleanup( void );

/* convenience */

#endif
