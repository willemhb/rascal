#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "rlvm.h"
#include "rlio.h"
#include "array.h"
#include "memory.h"
#include "compile.h"
#include "opcodes.h"

// external
// control implementation
Control *makeControl( Void )
{
  return (Control*)create( &ControlType );
}

Int freeControl( Control *control )
{
  deallocate(control, sizeof(Control));

  return 0;
}

Void initControl( Control *control, ByteCode *code )
{
  control->code = code;
  control->pc   = code->code->data;
}

// error handling
Bool recover( Void )
{
  Bool out = TheVm.panic;

  TheVm.panic = false;

  if (TheVm.error->count)
    {
      rlPrints(&Errs, TheVm.error->data);
      resetBuffer(TheVm.error);
    }

  return out;
}

Void panic( const Char *fmt, ... )
{
  TheVm.panic = true;

  if ( fmt != NULL )
    {
      FILE *tmp rl_cleanup(cleanupFILE) = open_s(tmpfile);
      va_list va; va_start(va, fmt);

      Size n = fprintf(tmp, fmt, va); va_end(va);

      resizeBuffer(TheVm.error, n);
      fgets(TheVm.error->data, n, tmp);
    }
}

// exec implementation & helpers

Value rlExec( ByteCode *bytecode )
{
  static Void *labels[] =
    {
      [OpNothing] = &&labelNothing,
      [OpConstant]= &&labelConstant,
      [OpHalt]    = &&labelHalt
    };

  OpCode op;

  Short argx, argy;

  

 labelHalt:
  
}

// initialization
Void rlVmInit( Void )
{
  TheVm.panic = false;
  TheVm.error = makeBuffer(); initBuffer(TheVm.error);
}

// globals
Vm TheVm =
  {
    {
      .dtype=&VmType.obj
    },

    .panic = false,
    .error = NULL
  };

Type VmType =
  {
    {
      .dtype=&TypeType.obj
    },

    "vm",
    sizeof(Vm)
  };

Type ControlType =
  {
    {
      .dtype=&TypeType.obj
    },

    "control",
    sizeof(Control)
  };
