#ifndef rl_vm_obj_reader_h
#define rl_vm_obj_reader_h

#include "rascal.h"
#include "vm/obj/support/ascii_buffer.h"
#include "vm/obj/support/readtable.h"

/* commentary */

/* C types */
typedef enum readstate_t
  {
   readstate_ready=0,
   readstate_expr =1,
   readstate_eos  =2,
   readstate_error=3,
  } readstate_t;

struct reader_t
{
  stream_t        stream;
  ascii_buffer_t *buffer;
  readtable_t    *readtable;
  readstate_t     readstate;
  value_t         value;
};

/* globals */
extern reader_t Reader;

/* API */
void reset_reader( reader_t *reader, stream_t stream );
void clear_reader( reader_t *reader );

/* runtime */
void rl_vm_obj_reader_init( void );
void rl_vm_obj_reader_mark( void );

/* convenience */

#endif
