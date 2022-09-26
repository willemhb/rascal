#ifndef rascal_rt_h
#define rascal_rt_h

#include <setjmp.h>

#include "obj.h"
#include "exec.h"
#include "obj/str.h"
#include "obj/func.h"
#include "obj/code.h"

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
#define error(agitant, fmt, ...)				\
  ({								\
    Agitant = agitant;						\
    char *_fn_ = Vm.code->func->name->data;			\
    fprintf( stderr,						\
	     "%s: error: "fmt".\n",				\
      	     _fn_ __VA_OPT__(,)					\
	     __VA_ARGS__);					\
    if (Catch)							\
      longjmp(Catch->saved, 1);					\
    else							\
      longjmp(Toplevel, 1);					\
  })

#define type_error(agitant, expected)				\
  ({								\
    val_t  _agitant_  = agitant;				\
    type_t _expected_ = expected;				\
    type_t _got_ = typeof_val(_agitant_);			\
    error(_agitant_,						\
	  "wanted a value of type %s, got a value of type %s",	\
	  ValApis[_expected_].name,				\
	  ValApis[_got_].name );				\
  })

#define require(condition, ...)			\
  do {						\
    if (!(condition))				\
      {						\
	error(__VA_ARGS__);			\
      }						\
  } while (0)

#endif
