#include <stdlib.h>
#include <stdio.h>
#include <stdalign.h>

#include "obj/obj.h"
#include "vm/vm.h"
#include "rl/rl.h"

// entry point, version information
#define VMAJOR     0
#define VMINOR     0
#define VPATCH     0
#define VDEV       "a"
#define VFMT       "%d.%d.%d.%s"

void rl_welcome( void )
{
  printf( "Welcome to rascal version "VFMT"!\n", VMAJOR, VMINOR, VPATCH, VDEV );
}

void rl_goodbye( void )
{
  printf( "Exiting normally.\n" );
}

void rl_init( void )
{
  rl_vm_init();
  rl_obj_init();
  rl_rl_init();
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
