#include <stdio.h>

#include "rlio.h"
#include "value.h"
#include "number.h"

// internal forward declarations
void printList( Port *ios, Value x );

// implementations
Void rlPrint( Port *ios, Value x )
{
  Type *xType = rlTypeof(x);

  if ( xType == &ConsType || xType == &NulType )
    printList(ios, x);

  else if ( xType == &FixnumType )
    fprintf(ios->stream, "%lu", asFixnum(x));

  else if ( xType == &RealType )
    fprintf(ios->stream, "%.2f", asReal(x));

  else
    fprintf(ios->stream, "<%s>", xType->name);
}

// specializations
Void printList( Port *ios, Value x )
{
  fprintf(ios->stream, "(");

  while ( isCons(x) )
    {
      Value ca = asCons(x)->car;

      rlPrint(ios, ca);

      x = asCons(x)->cdr;

      if ( isCons(x) )
	fprintf(ios->stream, " ");

      else if ( !isNul(x) )
	{
	  fprintf(ios->stream, " . ");
	  rlPrint(ios, x);
	}
    }

  fprintf(ios->stream, ")");
}

// initialization
Void rlioInit( Void )
{
  initPort( &ins, stdin );
  initPort( &outs, stdout );
  initPort( &errs, stderr );
}

// globals
Port ins =
  {
    {
      .dtype = &PortType.obj
    },

    NULL
  };

Port outs =
  {
    {
      .dtype = &PortType.obj
    },

    NULL
  };

Port errs =
  {
    {
      .dtype = &PortType.obj
    },

    NULL
  };

