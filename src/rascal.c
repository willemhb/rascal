#include "value.h"
#include "memory.h"
#include "object.h"
#include "number.h"
#include "rlio.h"

Void initRl( Void ) {}

Int main(const Int argc, const Char *argv[])
{
  (Void)argc;
  (Void)argv;

  Value x = rlFixnum( 1 ), y = rlReal( 1.2 ), z = rlNul;

  Value list = listn( 3, x, y, z );

  rlPrint( &outs, list );

  deallocate( asObject(list) );

  return 0;
}
