#include <stdlib.h>
#include <stdio.h>

#include "common.h"

// value types and tags
typedef double real_t;
typedef uword  value_t;

typedef union
{
  value_t as_tagged;
  real_t  as_real;
} rl_data_t;

#define QNAN  0x7ffc000000000000ul

#define as_value( x ) (((rl_data_t)(x)).as_tagged)

static inline bool   is_real( value_t x ) { return (x&QNAN) != QNAN; }
static inline real_t as_real( value_t x ) { return ((rl_data_t)x).as_real; }

// print
void rl_print( value_t x )
{
  if (is_real(x))
    printf("%.2f", as_real(x));
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
  
  rl_print(as_value(real_val));
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
