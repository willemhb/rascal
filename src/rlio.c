#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "rlio.h"
#include "value.h"
#include "number.h"
#include "chrnames.h"
#include "memory.h"
#include "rlvm.h"

// internal C types
typedef Void (*ReaderFn)(Port *port, Int dispatch);

// internal forward declarations
Void printList( Port *port, Value x );
Bool isRlSpace( Int dispatch );
Bool isRlSep( Int dispatch );

// implementations
// port object implementation
Port *createPort( Void )
{
  return (Port*)create(&PortType);
}

Int releasePort( Port *port )
{ 
  if ( !(port->flags & PortFlStd) )
    fclose(port->stream);

  releaseBuffer(port->buffer);

  return deallocate(port, sizeof(Port));
}

Void initPort( Port *port, FILE *stream, PortFlag fl )
{
  port->stream = stream;
  port->flags  = fl;
  port->buffer = newBuffer();
}

Port *newPort( FILE *stream, PortFlag fl )
{
  Port *out = createPort();
  initPort(out, stream, fl);

  return out;
}

// port IO api
Bool isEOS( Port *port )
{
  return feof(port->stream);
}

Int rlPeekc( Port *port )
{
  Int ch = fgetc(port->stream);

  if (ch != EOF)
    ungetc(ch, port->stream);

  return ch;
}

Int rlReadc( Port *port )
{
  return fgetc(port->stream);
}

Void rlTakec( Port *port )
{
  fgetc(port->stream);
}

Int rlPrintc( Port *port, Ascii character )
{
  if ( isEOS(port) )
    return EOF;

  return fputc(character, port->stream);
}

Int rlPrints( Port *port, Ascii *string )
{
  if ( isEOS(port) )
    return EOF;

  return fputs(string, port->stream);
}

Int rlPrintf( Port *port, const Char *fmt, ... )
{
  if ( isEOS(port) )
    return EOF;

  va_list va; va_start(va, fmt);
  Int out = vfprintf(port->stream, fmt, va);

  return out;
}

// reader & read implementations
Size accumulate( Port *port, Ascii character )
{
  pushToBuffer(port->buffer, character);

  return port->buffer->count;
}

// read implementation
Value rlRead( Port *port )
{
  return port->value;
}

// print implementation
Void rlPrint( Port *port, Value x )
{
  Type *xType = rlTypeof(x);

  if ( xType == &ConsType || xType == &NulType )
    printList(port, x);

  else if ( xType == &FixnumType )
    rlPrintf(port, "%lu", asFixnum(x));

  else if ( xType == &RealType )
    rlPrintf(port, "%.2f", asReal(x));

  else if ( xType == &AsciiType )
    {
      if ( x == rlEOS )
	rlPrintf(port, "\\eos");

      else
	rlPrintf(port, "\\%s", ChrNames[(Size)asAscii(x)]);
    }

  else
    rlPrintf(port, "<%s>", xType->name);
}

// print specializations
Void printList( Port *port, Value x )
{
  rlPrintf(port, "(");

  while ( isCons(x) )
    {
      Value ca = asCons(x)->car;

      rlPrint(port, ca);

      x = asCons(x)->cdr;

      if ( isCons(x) )
	rlPrintf(port, " ");

      else if ( !isNul(x) )
	{
	  rlPrintf(port, " . ");
	  rlPrint(port, x);
	}
    }

  rlPrintf(port, ")");
}

// initialization
Void rlioInit( Void )
{
  initPort( &ins, stdin, ins.flags );
  initPort( &outs, stdout, outs.flags );
  initPort( &errs, stderr, errs.flags );
}

// globals
Type PortType =
  {
    {
      .dtype=&TypeType.obj
    },

    "port",
    sizeof(Port)
  };

Port ins =
  {
    {
      .dtype = &PortType.obj
    },

    PortFlInput|PortFlText|PortFlStd,
    false,
    NULL,
    NULL
  };

Port outs =
  {
    {
      .dtype = &PortType.obj
    },

    PortFlOutput|PortFlText|PortFlStd,
    false,
    NULL,
    NULL
  };

Port errs =
  {
    {
      .dtype = &PortType.obj
    },

    PortFlOutput|PortFlText|PortFlStd,
    false,
    NULL,
    NULL
  };

