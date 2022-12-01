#include "obj/real.h"
#include "obj/type.h"

/* commentary */

/* C types */

/* globals */
struct vtable_t RealMethods =
  {
    NULL, NULL, NULL
  };

struct layout_t RealLayout =
  {
    .vmtype=vmtype_flo64,
    .obsize=0
  };

struct datatype_t RealType =
  {
    {
      .obj=obj_init(&TypeType, sizeof(datatype_t), object_fl_static),
      .name="real"
    },

    .layout=&RealLayout,
    .methods=&RealMethods
  };

/* API */
#define real_op(name, op)			\
  value_t name##_reals( value_t x, value_t y )	\
  {						\
    return as_value(as_real(x) op as_real(y));	\
  }

#define real_pred(name, op)				\
  value_t name##_reals( value_t x, value_t y )		\
  {							\
    return (as_real(x) op as_real(y)) ? TRUE : FALSE;	\
  }

real_op(add, +);
real_op(sub, -);
real_op(mul, *);
real_op(div, /);
real_pred(eq, ==);
real_pred(lt, <);
real_pred(gt, <);
real_pred(leq, >=);
real_pred(geq, <=);
real_pred(neq, !=);

/* runtime */
void rl_obj_real_init( void )
{
  gl_init_type(RealType);
}

void rl_obj_real_mark( void )
{
  gl_mark_type(RealType);
}

void rl_obj_real_cleanup( void ) {}

/* convenience */
