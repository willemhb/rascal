#ifndef read_h
#define read_h

#include <stdio.h>

#include "common.h"
#include "runtime.h"

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
value_t read( FILE* ios );
void reset_reader( struct Reader* reader );
void init_reader( struct Reader* reader, FILE* infile );

// misc -----------------------------------------------------------------------
void reader_init( void );

#endif
