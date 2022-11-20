#ifndef rascal_rlio_h
#define rascal_rlio_h

#include "table.h"

// C types
typedef struct Port
{
  OBJECT

  FILE *stream;
} Port;

// forward declarations
// port implementation
Port *createPort( Void );
Void  initPort( Port *created, FILE *stream );
Port *newPort( FILE *stream );

#define asPort( x ) ((Port*)asObject(x))
#define isPort( x ) valueIsType(x, &PortType)

// print implementation
Void rlPrint( Port *ios, Value x );

// initialization
Void rlioInit( Void );

// globals
extern Type PortType;

extern Port ins, outs, errs;

extern ReadTable Reader;

#endif
