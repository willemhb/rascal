#ifndef rl_tpl_impl_array_h
#define rl_tpl_impl_array_h

#define MAKEARR(T)				\
  _Type *make##_Type( Void )			\
  {						\
    return (_Type*)create(&_Type##Type);	\
  }

#define FREEARR(_Type, _ElType)					\
  Int free##_Type( _Type *array )				\
  {								\
    deallocArray(array->data, array->count, sizeof(_ElType));	\
    deallocate(array, sizeof(_Type));				\
    return 0;							\
  }

#define INITARR(_Type, _ElType)					\
  Void init##_Type( _Type *array )				\
  {								\
    array->count = 0;						\
    array->capacity = minCap;					\
    array->data = allocArray(array->capacity, sizeof(_ElType));	\
  }

#define RESIZEARR(_Type, _ElType, isString)				\
  Size resize##_Type( _Type *array, Size newCount )			\
  {									\
    Size oldCap = array->capacity,					\
         newCap = padArrayLength(newCount+isString, oldCap);		\
    if ( oldCap != newCap )						\
      array->data = reallocArray(array->data, newCap, sizeof(_ElType));	\
    array->count = newCount;						\
    array->capacity = newCap;						\
    return newCount;							\
  }

#define GETFROMARR(_Type, _ElType)			\
  _ElType getFrom##_Type( _Type *array, Long i )	\
  {							\
    if ( i < 0 )					\
      i += array->count;				\
    assert(i >= 0 && (Size)i < array->count);		\
    return array->data[i];				\
  }

#define SETINARR(_Type, _ElType)				\
  _Type *setIn##_Type( _Type *array, Long i, _ElType x )	\
  {								\
    if ( i < 0 )						\
      i += array->count;					\
    assert(i >= 0 && (Size)i < array->count);			\
    array->data[i] = x;						\
    return array;						\
  }

#define POPFROMARR(_Type, _ElType)			\
  _ElType popFrom##_Type( _Type *array, Size n )	\
  {							\
    assert( n <= array->count);				\
    _ElType out = array->data[array->count-1];		\
    resize##_Type(array, array->count-n);		\
    return out;						\
  }

#define APPENDTOARR(_Type, _ElType, _VaType, isString)			\
  Size appendTo##_Type( _Type *array, Size n, ... )			\
  {									\
    resize##_Type(array, array->count+n);				\
    _ElType buffer[n+isString];						\
    va_list va; va_start(va, n);					\
    for (Size i=0; i<n; i++)						\
      buffer[i] = va_arg(va, _VaType);					\
    va_end(va);								\
    if (isString)							\
      buffer[n] = (_ElType)0;						\
    memcpy(array->data+n, buffer, (n+isString)*sizeof(_ElType));	\
    return array->count;						\
  }


#endif
