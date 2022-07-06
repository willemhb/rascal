#ifndef describe_array_h
#define describe_array_h

#define ArrayType(_dtype, _flags, _min)		  \
  {						  \
    object_t base;				  \
    size_t   len       : 56;			  \
    size_t   ctype     :  8;			  \
    size_t   cap       : 56;			  \
    size_t   inlined   :  1;			  \
    size_t   shared    :  1;			  \
    size_t   protected :  1;			  \
    size_t   boxed     :  1;			  \
    size_t   _flags    :  4;			  \
    union {					  \
      _dtype  space[_min];			  \
      _dtype *data;				  \
    };						  \
  }

#define DeclareArraySize(_type)						\
  size_t sizeofval##_type(value_t arrob, bool_t padded, bool_t inlined); \
  size_t sizeofob##_type(object_t *arrob, bool_t padded, bool_t inlined); \
  size_t sizeof##_type##_type(_type##_t* arrob, bool_t padded, bool_t inlined)

#define GetArraySize(_type)						\
  size_t sizeofval##_type(value_t arrob, bool_t padded, bool_t inlined)	\
  {									\
    return sizeof##_type##_type(to##_type(arrob), padded, inlined);	\
  }									\
  size_t sizeofob##_type(object_t *arrob, bool_t padded, bool_t inlined) \
  {									\
    return sizeof##_type##_type(to##_type(arrob), padded, inlined);	\
  }									\
  size_t sizeof##_type(_type##_t* arrob, bool_t padded, bool_t inlined)	\
  {									\
    size_t out = sizeof(_type##_t) - 8;					\
    size_t elsize = Ctype_sizeof(array->ctype);				\
    if (padded) out += array->cap * elsize;				\
    else if (array->inlined || inlined) out += array->len * elsize;	\
    else out += 8;							\
    return out;								\
  }

#define DeclareArrayData(_type, _eltype)	\
  _eltype val##_type##data( value_t val );	\
  _eltype ob##_type##data( object_t *ob );	\
  _eltype _type##_type##data( _type##_t *arr )

#define GetArrayData(_type, _eltype)			\
  _eltype *val##_type##data( value_t val )		\
  {							\
    return _type##_type##data( to##_type( val ) );	\
  }							\
  							\
  _eltype *ob##_type##data( object_t *ob )		\
  {							\
    return _type##_type##data( to##_type( ob ) );	\
  }							\
  							\
  _eltype *_type##_type##data(_type##_t *arr )		\
  {							\
    							\
    return arr->inlined ? &array->space[0] : arr->data;	\
  }

#define GenericArrayData(_type, _x)		\
  _Generic((_x),				\
	   value_t:val##_type##data,		\
	   object_t*:ob##_type##data,		\
	   _type##_t*:_type##_type##data)(_x)

#define GenericArraySize(_type, _x, _padded, _inlined)			\
  _Generic((_x),							\
	   value_t:sizeofval##_type,					\
	   object_t*:sizeofob##_type,					\
	   _type##_t*:sizeof##_type					\
	   )(_x, _padded, _inlined)

#endif
