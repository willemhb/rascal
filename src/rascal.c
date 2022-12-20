#include "rascal.h"

#include "val.h"
#include "obj.h"
#include "interp.h"
#include "reader.h"

// entry point, version information
#define VMAJOR     0
#define VMINOR     0
#define VPATCH     1
#define VDEV       "a"
#define VFMT       "%d.%d.%d.%s"

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
  interp_init();
  val_init();
  obj_init();
  reader_init();
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
