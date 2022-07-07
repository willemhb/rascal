#ifndef rascal_value_h
#define rascal_value_h

#include "rascal.h"

// api ------------------------------------------------------------------------
bool_t  _valtoCbool( value_t val );
bool_t  _obtoCbool( object_t *ob );

#define rtoCbool(x)					\
  _Generic((x),						\
	     value_t:_valtoCbool,			\
	   object_t*:_obtoCbool)(x)

type_t  _obtypeof(object_t *ob);
type_t  _valtypeof(value_t  val);

#define rtypeof(x)				\
  _Generic((x),					\
	   value_t:_valtypeof,			\
	   object_t*:_obtypeof)(x)

size_t  _valsizeof(value_t val);
size_t  _obsizeof(object_t *ob);
size_t  _typesizeof(type_t xt);

#define rsizeof(x)				\
  _Generic((x),					\
	   value_t:_valsizeof,			\
	   type_t: _typesizeof,			\
	   object_t*:_obsizeof)(x)

hash_t  _valhash(value_t val);
hash_t  _obhash(object_t *ob);

#define rhash(x)				\
  _Generic((x),					\
	   value_t:_valhash,			\
	   object_t*:_obhash)(x)

char_t *_typetypename(type_t xt);
char_t *_valtypename(value_t val);
char_t *_obtypename(object_t *ob);

#define rtypename(x)					\
  _Generic((x),						\
	   value_t:_valtypename,			\
	   type_t: _typetypename,			\
	   object_t*:_obtypename)(x)

bool_t  arrayp(value_t xv);
bool_t  sequencep(value_t xv);

#endif
