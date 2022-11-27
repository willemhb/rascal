#include <stdlib.h>
#include <stdio.h>
#include <stdalign.h>

#include "object.h"
#include "memory.h"


// entry point, version information
#define VMAJOR     0
#define VMINOR     0
#define VPATCH     0
#define VDEV       "a"
#define VFMT       "%d.%d.%d.%s"
#define PROMPT     ">>>"

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
  
}

void rl_main( void )
{
  real_t real_val = 12.4;
  
  rl_println(as_value(real_val));
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
