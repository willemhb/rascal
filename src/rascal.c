#include <stdlib.h>
#include <stdio.h>
#include <stdalign.h>

#include "object.h"
#include "memory.h"


typedef union
{
  value_t   as_tagged;
  real_t    as_real;
  object_t *as_object;
  type_t   *as_type;
  cons_t   *as_cons;
  atom_t      *as_atom;
  primitive_t *as_primitive;
  lambda_t    *as_lambda;
  vector_t    *as_vector;
  string_t *as_string;
  stream_t *as_stream;
} rl_data_t;

#define QNAN    0x7ff8000000000000ul

#define TMASK   0xffff000000000000ul

#define NUL     0x7ffc000000000000ul
#define TRUE    0x7ffd000000000001ul
#define FALSE   0x7ffd000000000000ul
#define GLYPH   0x7ffe000000000000ul
#define FIXNUM  0x7fff000000000000ul
#define SMINT   0xfffc000000000000ul
#define PFUNC   0xfffd000000000000ul
#define STREAM  0xfffe000000000000ul
#define OBJECT  0xffff000000000000ul

static inline bool is_real( value_t x )     { return (x&QNAN) != QNAN; }
static inline bool is_nul( value_t x )      { return x == NUL; }
static inline bool is_bool( value_t x )     { return (x&TMASK) == FALSE; }
static inline bool is_glyph( value_t x )    { return (x&TMASK) == GLYPH; }
static inline bool is_fixnum( value_t x )   { return (x&TMASK) == FIXNUM; }
static inline bool is_smint( value_t x )    { return (x&TMASK) == SMINT; }
static inline bool is_pfunc( value_t x )    { return (x&TMASK) == PFUNC; }
static inline bool is_stream( value_t x )   { return (x&TMASK) == STREAM; }

#define as_value( x )  (((rl_data_t)(x)).as_tagged)
#define as_object( x ) (((rl_data_t)(x)).as_object)
#define as_type( x )   (((rl_data_t)(x)).as_type)
#define as_real( x )   (((rl_data_t)(x)).as_real)
#define as_cons( x )   (((rl_data_t)(x)).as_cons)
#define as_atom( x )   (((rl_data_t)(x)).as_atom)
#define as_pfun( x )   (((rl_data_t)(x)).as_pfun)
#define as_lfun( x )   (((rl_data_t)(x)).as_lfun)
#define as_vector( x ) (((rl_data_t)(x)).as_vector)
#define as_string( x ) (((rl_data_t)(x)).as_string)

// virtual machine
typedef enum
  {
    op_nothing=1,  // noop

    /* load/store instructions */
    op_constant=2, // load from constant store
  } opcode_t;

value_t rl_exec( lfun_t *code )
{
  static void *labels[] =
    {
      [0] = &&label_invalid,
    };

  ushort op = 0;

  short  argx, argy;

  value_t v, x, y;

  

 label_fetch:

  label_

 label_invalid:
  fprintf(stderr, "error: invalid opcode %d.\n", op);
  fprintf(stderr, "aborting.\n");
  abort();

  
}

// print
void rl_print( value_t x )
{
  if (is_real(x))
    printf("%.2f", as_real(x));
}

void rl_println( value_t x )
{
  rl_print( x );
  printf("\n");
}

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

void rl_init(  void ) {}

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
