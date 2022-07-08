#ifndef describe_array_h
#define describe_array_h

// declaration templates ------------------------------------------------------
#define DeclareArraySize(__t)					\
  size_t vasize_##__t( value_t x, bool_t p );			\
  size_t oasize_##__t( object_t *x, bool_t p );			\
  size_t __t##_asize##__t( __t##_t* x, bool_t p )

#define DeclareArrayData( __t, __e )		\
  __e *vdata_##__t( value_t x );		\
  __e *odata_##__t( object_t *x );		\
  __e *__t##data_##__t( __t##_t *x )

#define DeclareArrayParent( __t )		\
  object_t *vparent_##__t( value_t x );		\
  object_t *oparent_##__t( object_t *x );	\
  object_t *__t##parent_##__t( __t##_t* x )

#define DeclareArrayRef( __t, __e )				\
  __e vref_##__t( value_t x, int_t n );				\
  __e oref_##__t( object_t *x, int_t n );			\
  __e __t##ref_##__t( __t##_t *x, int_t n )

#define DeclareArrayXef( __t, __e )					\
  value_t   vxef_##__t( value_t x, bool_t c, int_t n, __e v );		\
  object_t *oxef_##__t( object_t *x, bool_t c, int_t n, __e v );	\
  __t##_t  *__t##xef_##__t( __t##_t *x, bool_t c, int_t n, __e v )

#define DeclareArrayPut( __t, __e )				\
  value_t   vput_##__t( value_t x, __e v );			\
  object_t *oput_##__t( object_t *x, __e v );			\
  __t##_t  *__t##put_##__t( __t##_t *x, __e v )

#define DeclareArrayXut( __t, __e )				\
  value_t   vxut_##__t( value_t x, bool_t c, __e v );		\
  object_t* oxut_##__t( object_t *x, bool_t c, __e v );		\
  __e __t##xut_##__t( __t##_t *x, bool_t c, __e v )

#define DeclareArrayResize( __t )				\
  value_t   vresize_##__t( value_t   x, size_t n );		\
  object_t* oresize_##__t( object_t *x, size_t n );		\
  __t##_t*  __t##resize_##__t( __t##_t *x, size_t n )

// implementation templates ---------------------------------------------------
#define DescribeArraySize(__t)						\
  size_t vasize_##__t( value_t x, bool_t p )				\
  {									\
    return __t##_asize##__t( to##__t(x), p );				\
  }									\
  size_t oasize##__t(object_t *x, bool_t p )				\
  {									\
    return __t##_asize##__t( to##__t(x), p );				\
  }									\
  size_t __t##_asize##__t(__t##_t* x, bool_t p )			\
  {									\
    if (p)								\
      return Ctype_size( x->ctype ) * x->len;				\
    else								\
      return Ctype_size( x->ctype ) * x->len;				\
  }

#define DescribeArrayData( __t, __e )			\
  __e *vdata_##__t( value_t val )			\
  {							\
    return __t##data_##__t( to##__t( val ) );		\
  }							\
  __e *odatab##__t( object_t *ob )			\
  {							\
    return __t##data_##__t( to##__t( ob ) );		\
  }							\
  							\
  __e *__t##data_##__t(__t##_t *arr )			\
  {							\
    return arr->inlined ? &array->space[0] : arr->data;	\
  }

#define DescribeArrayParent( __t )		\
  object_t *vparent_##__t( value_t x )		\
  {						\
    return __t##parent_##__t( to##__t( x ) );	\
  }						\
  object_t *oparent_##__t( object_t *x )	\
  {						\
    return __t##parent_##__t( to##__t( x ) );	\
  }						\
  object_t *__t##parent_##__t( __t##_t* x )	\
  {						\
    if ( !!( oblfags( x ) & arrfl_shared) )	\
      return ((object_t**)ptr( x ))[3];		\
    return NULL;				\
  }

#define DescribeArrayRef( __t, __e )			\
  __e vref_##__t( value_t x, int_t i )			\
  {							\
    return __t##ref_##__t( to##__t( x ), int_t );	\
  }							\
  __e oref_##__t( object_t *o, int_t i )		\
  {							\
    return __t##ref_##__t( to##__t( x ), i );		\
  }							\
  __e __t##ref_##__t( to##__t* x, int_t i )		\
  {							\
    if (i < 0)						\
      i += x->len;					\
    require( i > 0 && i < x->len,			\
	     (value_t)x,				\
	     "%d out of bounds for size %d",		\
	     i,						\
	     x->len );					\
    return __t##data_##__t( x )[ i ];			\
  }

#define DescribeArrayXef( __t, __e )					\
  value_t vxef_##__t( value_t x, bool_t c, int_t i, __e v)		\
  {									\
    return __t##xef_##__t( to##__t( x ), c, i, v );			\
  }									\
  object_t *oxef_##__t( object_t *o, bool_t c, int_t i, __e v)		\
  {									\
    return __t##xef_##__t( to##__t( x ), i );				\
  }									\
  __t##_t *__t##xef_##__t( to##__t* x, bool_t c, int_t i, __e v)	\
  {									\
    if (i < 0)								\
      i += x->len;							\
    require( i > 0 && i < x->len,					\
	     (value_t)x,						\
	     "%d out of bounds for size %d",				\
	     i,								\
	     x->len );							\
    if ( protectp( (object_t*)x ) && !c )				\
      x = mk_copy( (object_t*)x );					\
    __t##data_##__t( x )[ i ];						\
    return x;								\
  }

#define DescribeArrayPut( __t, __e )					\
  value_t vput_##__t( value_t x, __e v)					\
  {									\
    return (value_t)__t##put_##__t( to##__t( x ), v );			\
  }									\
  object_t *oput_##__t( object_t *o, __e v)				\
  {									\
    return (object_t*)__t##put_##__t( to##__t( x ), v );		\
  }									\
  __t##_t *__t##put_##__t( __t##_t* x, __e v)				\
  {									\
    size_t								\
    if ( x->len + 1 == x->cap )						\
      									\
    x = (__t##_t*);							\
  }

#define DescribeArrayXut( __t, __e )					\
  value_t vxef_##__t( value_t x, bool_t c, int_t i, __e v)		\
  {									\
    return __t##xef_##__t( to##__t( x ), c, i, v );			\
  }									\
  object_t *oxef_##__t( object_t *o, bool_t c, int_t i, __e v)		\
  {									\
    return __t##xef_##__t( to##__t( x ), i );				\
  }									\
  __t##_t *__t##xef_##__t( to##__t* x, bool_t c, int_t i, __e v)	\
  {									\
    if (i < 0)								\
      i += x->len;							\
    require( i > 0 && i < x->len,					\
	     (value_t)x,						\
	     "%d out of bounds for size %d",				\
	     i,								\
	     x->len );							\
    if ( protectp( (object_t*)x ) && !c )				\
      x = mk_copy( (object_t*)x );					\
    __t##data_##__t( x )[ i ];						\
    return x;								\
  }

#define DescribeArrayResize( __t, __e, _unboxed, _string )	\
  value_t vresize_##__t( value_t x, size_t n )			\
  {								\
    return (value_t)__t##resize_##__t( to##__t( x ), n );	\
  }								\
  object_t* oresize_##__t( object_t *x, size_t n )		\
  {									\
    return (object_t*)__t##resize_##__t( to##__t( x ), n );		\
  }									\
  __t##_t *__t##resize_##__t( __t##_t* x, size_t n )			\
  {									\
    size_t olen = x->len, ocap = x->cap;				\
    x->len = n;								\
    x->cap = arr_resize( n + _string );					\
    if ( n > ocap || n > ocap / 2)					\
      x = resize( (object_t*)x, 2 );					\
    return x;								\
  }									\


// generic template -----------------------------------------------------------
#define GenericArrayData( __t, _x )		\
  _Generic((_x),				\
	   value_t:vdata_##__t,			\
	   object_t*:odata_##__t,		\
	   __t##_t*:__t##data_##__t)( _x )

#define GenericArrayParent( __t, _x )		\
  _Generic((_x),				\
	   value_t:vparent_##__t,		\
	   object_t*:oparent_##__t,		\
	   __t##_t*:__t##parent_##__t )( _x )

#define GenericArraySize( __t, _x, _p )					\
  _Generic((_x),							\
	   value_t:vasize_##__t,					\
	   object_t*:oasize_##__t,					\
	   __t##_t*:__t##asize_##__t )( _x, _p )

#define GenericArrayRef( __t, _x, _k )		\
  _Generic((_x),				\
	   value_t:vref_##__t,			\
	   object_t*:oref_##__t,		\
	   __t##_t*:__t##ref_##__t)(_x, _k)

#define GenericArrayXef( __t, _x, _c, _k, _v )				\
  _Generic((_x),							\
	   value_t:vxef_##__t,						\
	   object_t*:oxef_##__t,					\
	   __t##_t*:__t##xef_##__t )( _x, _c, _k, _v )

#define GenericArrayPut( __t, _x, _v )			\
  _Generic((_x),					\
	   value_t:vput_##__t,				\
	   object_t*:oput_##__t,			\
	   __t##_t*:__t##put_##__t)( _x, _v )

#define GenericArrayXut( __t, _x, _c, _v )				\
  _Generic((_x),							\
	   value_t:vxut_##__t,						\
	   object_t*:oxut_##__t,					\
	   __t##_t*:__t##xut_##__t )( _x, _c, _v )

#endif
