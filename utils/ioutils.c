#include "ioutils.h"


int fpeekc( FILE *fl ) {
  int out = fgetc( fl );

  if ( out != EOF )
    ungetc( out, fl );

  return out;
}
