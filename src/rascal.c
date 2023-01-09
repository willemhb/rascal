#include "rascal.h"

/* runtime */
#include "val.h"
#include "obj.h"
#include "type.h"
#include "memory.h"
#include "reader.h"
#include "vm.h"

/* object system */
#include "real.h"
#include "sym.h"
#include "list.h"
#include "vec.h"
#include "code.h"
#include "prim.h"
#include "native.h"
#include "module.h"

/* interpreter */
#include "read.h"
#include "eval.h"
#include "prin.h"
#include "exec.h"
#include "comp.h"
#include "repl.h"

// entry point, version information
#define VMAJOR     0
#define VMINOR     0
#define VPATCH     8
#define VDEV       "a"
#define VFMT       "%d.%d.%d.%s"

// important symbols
val_t QuoteSym, DoSym, CatchSym, RaiseSym, PutSym, DefSym, IfSym, FunSym;

void rl_welcome( void )
{
  printf("Welcome to rascal version "VFMT"!\n", VMAJOR, VMINOR, VPATCH, VDEV);
}

void rl_goodbye( void )
{
  printf("Exiting normally.\n");
}

void rl_init( void )
{
  // sym init (symbol table initialized here, must be first)
  extern void sym_init(void);    sym_init();
  
  // object model initializers
  extern void arr_init(void);    arr_init();
  extern void bool_init(void);   bool_init();
  extern void code_init(void);   code_init();
  extern void func_init(void);   func_init();
  extern void list_init(void);   list_init();
  extern void module_init(void); module_init();
  extern void native_init(void); native_init();
  extern void num_init(void);    num_init();
  extern void prim_init(void);   prim_init();
  extern void real_init(void);   real_init();
  extern void small_init(void);  small_init();
  extern void vec_init(void);    vec_init();

  // vm initializers
  extern void reader_init(void); reader_init();
  extern void vm_init(void);     vm_init();
  extern void comp_init(void);   comp_init();
  extern void val_init(void);    val_init();
}

void rl_main( void )
{
  repl();
}

void rl_post( void ) {}

int main(const int argc, const char *argv[])
{
  (void)argc;
  (void)argv;

  rl_init();
  rl_welcome();
  rl_main();
  rl_goodbye();
  rl_post();

  return 0;
}
