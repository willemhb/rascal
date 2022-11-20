#include "value.h"
#include "memory.h"
#include "object.h"
#include "number.h"
#include "rlio.h"

#define VMAJOR     0
#define VMINOR     0
#define VPATCH     0
#define VDEV       "a"
#define VFMT       "%d.%d.%d.%s"
#define PROMPT     ">>>"

void printWelcome( Void )
{
  printf( "Welcome to rascal version "VFMT"!\n", VMAJOR, VMINOR, VPATCH, VDEV );
}

void printPrompt( Void )
{
  printf( PROMPT" " );
}

void printNewline( Void )
{
  printf( "\n" );
}

Void initRl( Void )
{
  rlioInit();
  printWelcome();
}

Int main(const Int argc, const Char *argv[])
{
  (Void)argc;
  (Void)argv;

  initRl();

  Value x = rlFixnum( 1 ), y = rlReal( 1.2 ), z = rlNul;

  Value list = listn( 3, x, y, z );

  rlPrint( &outs, list );
  printNewline();
  deallocate( asObject(list) );

  return 0;
}
