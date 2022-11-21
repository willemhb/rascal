#ifndef rascal_rlio_h
#define rascal_rlio_h

#include "object.h"

/**
 *
 * Implementations for types used primarily for IO operations.
 *
 **/

// C types
typedef enum PortFlag PortFlag;
typedef enum ReadState ReadState;

typedef struct Port   Port;
typedef struct Buffer Buffer;

enum PortFlag
  {
    PortFlInput  = 1,
    PortFlOutput = 2,
    PortFlBinary = 4,
    PortFlText   = 8,
    PortFlStd    =16,
  };

enum ReadState
  {
    ReadReady     =0,
    ReadExpression=1,
    ReadEOS       =2,
  };

struct Port
{
  OBJECT

  PortFlag   flags;
  ReadState  state;

  FILE      *stream;
  Buffer    *buffer;
  Value      value;
};

// forward declarations
// general IO utilities
Void cleanupFILE( FILE **file );

// port implementation
Port *makePort( Void );
Int   freePort( Port *port );
Void  initPort( Port *created, FILE *stream, PortFlag fl );

// port IO api
Bool isEOS( Port *port );

Int  rlPeekc( Port *port );
Int  rlReadc( Port *port );
Void rlTakec( Port *port );
Int  rlPrintc( Port *port, Ascii character );
Int  rlPrints( Port *port, Ascii *string );
Int  rlPrintf( Port *port, const Char *fmt, ... );

#define asPort( x ) ((Port*)asObject(x))
#define isPort( x ) valueIsType(x, &PortType)

// initialization
Void rlioInit( Void );

// globals
extern Type PortType;

extern Port Ins, Outs, Errs;

// convenience
#define open_s( func, ... )			\
  ({						\
    FILE *__out__ = func(__VA_ARGS__);		\
    if ( __out__ == NULL )			\
      {						\
	fprintf(stderr,				\
		"%s failed at %s:%d in %s.\n",	\
		#func,				\
		__FILE__,			\
		__LINE__,			\
		__func__);			\
	abort();				\
      }						\
    __out__;					\
  })

#endif
