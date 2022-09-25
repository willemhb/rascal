#ifndef rascal_rt_h
#define rascal_rt_h

#include <setjmp.h>

#include "obj/obj.h"

// C types
typedef struct catch_frame_t
{
  struct catch_frame_t *next;

  // index of frame to restore
  arity_t fp;

  jmp_buf saved;
} catch_frame_t;

// globals
extern val_t Agitant;
extern catch_frame_t *Catch;
extern jmp_buf Toplevel;

// convenience
#define error(fname, agitant, fmt, ...)				\
  ({								\
    Agitant = agitant;						\
    fprintf( stderr,						\
	     fname,						\
	     "%s: error: "fmt".\n",				\
	     __VA_ARGS__);					\
    if (Catch)							\
      longjmp(Catch->saved, 1);					\
    else							\
      longjmp(Toplevel, 1);					\
  })

#define type_error(fname, agitant, expected)			\
  ({								\
    val_t  _agitant_  = agitant;				\
    type_t _expected_ = expected;				\
    type_t _got_ = typeof_val(_agitant_);			\
    error(fname,						\
	  _agitant_,						\
	  "wanted a value of type %s, got a value of type %s",	\
	  typename(_expected_),					\
	  typename(_got_));					\
  })

#define require(condition, ...)			\
  do {						\
    if (!(condition))				\
      {						\
	error(__VA_ARGS__);			\
      }						\
  } while (0)

#endif
