#ifndef rascal_describe_utils_h
#define rascal_describe_utils_h

/* frequently one wants to use a genericized operation as a function pointer,
   but the nature of generics prevents it. This is an attempt to solve that problem,
   by creating a simple macro to select a given method.
 */

#define methodfor(cnt, ...)      methodfor##_##cnt( ##__VA_ARGS__ )

#define methodfor_2( name, mt )			\
  _Generic((mt)0,				\
	   value_t:v##name,			\
	   object_t*:o##name)

#define methodfor_3( name, type, mt )		\
  _Generic((mt)0,				\
	   value_t:v##name##_##type,		\
	   object_t*:o##name##_##type,		\
	   type##_t*:type##name##_##type)

#endif
