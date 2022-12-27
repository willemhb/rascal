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
#define VPATCH     5
#define VDEV       "a"
#define VFMT       "%d.%d.%d.%s"

// important symbols
val_t QuoteSym, DoSym, CatchSym, PutSym, DefSym, IfSym, FunSym;

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
  extern void sym_init(void);
  extern void reader_init(void);
  extern void vm_init(void);
  extern void num_init(void);
  extern void cons_init(void);
  extern void vec_init(void);
  extern void code_init(void);
  extern void comp_init(void);

  sym_init();
  reader_init();
  vm_init();
  num_init();
  cons_init();
  vec_init();
  code_init();
  comp_init();
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
