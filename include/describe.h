#ifndef rascal_describe_h
#define rascal_describe_h

#define GENERIC_2( method, x, ... )					\
  _Generic((x),								\
	   value_t:val_##method,					\
	   object_t*:obj_##method)( (x) __VA_OPT__(,) __VA_ARGS__ )

#define GENERIC_3( type, method, x, ... )				\
  _Generic((x),								\
	   value_t:val_##method,					\
	   object_t*:obj_##method,					\
	   type##_t*:type##_##method)( (x) __VA_OPT__(,) __VA_ARGS__ )

#endif
