#ifndef rascal_describe_h
#define rascal_describe_h

#define GENERIC_2( t1, m1, t2, m2, x, ...)	\
  _Generic((x),					\
	   t1:m1,				\
	   t2:m2 )((x) __VA_OPT__(,) __VA_ARGS__)


#define GENERIC_3( method, x, ...)					\
  _Generic((x),								\
	   value_t:val_##method,					\
	   object_t*:obj_##method,					\
	   boxed_t*:box_##method)((x) __VA_OPT__(,) __VA_ARGS__)

#endif
