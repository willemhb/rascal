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
/* high level */
void    reset_reader( reader_t *reader, stream_t stream );
void    clear_reader( reader_t *reader );
int     give_value( reader_t *reader, value_t value );
value_t take_value( reader_t *reader );
int     set_status( reader_t *reader, readstate_t status, const char *fmt, ... );

/* buffer interface */
int accumulate_character( reader_t *reader, int character );

/* stream interface */
bool is_eos( reader_t *reader );
int  next_char( reader_t *reader );
int  peek_char( reader_t *reader );

/* readtable interface */
reader_dispatch_fn_t get_dispatch_fn( reader_t *reader, int character );
int  set_reader_macro( reader_t *reader, int dispatch, reader_dispatch_fn_t handler );
int  set_reader_macros( reader_t *reader, char *dispatches, reader_dispatch_fn_t handler );

/* runtime */
void rl_vm_obj_reader_init( void );
void rl_vm_obj_reader_mark( void );
void rl_vm_obj_reader_cleanup( void );

/* convenience */

#endif
