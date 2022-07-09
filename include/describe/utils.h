#ifndef rascal_describe_utils_h
#define rascal_describe_utils_h

#define collection_type(eltype)			\
  {						\
    value_t type;				\
    eltype *data;				\
    int len, cap;				\
  }

#define get( type, x, field ) (((type##_t*)asptr(x))->field)

#define mk_tag_p(t) inline bool t##p( value_t x ) { return x && vtag(x) == tag_##t; }

#define mk_type_p(t) inline bool t##p( value_t x ) { return val_typeof( x ) == type_##t; }

#define mk_type_dispatch(rtype, fname, tableName)			\
  inline rtype val_##fname( value_t x )					\
  {									\
    return tableName[val_typeof(x)];					\
  }

#define array_ref(type, eltype)			\
  eltype type##_ref(value_t x, int n )		\
  {						\
    int l = get( type, x, len );		\
    if ( n < 0 )				\
      n += l;					\
    assert( n > 0 && n < l );			\
    return ((eltype*)get( type, x, data ))[n];	\
  }

#define array_set(type, eltype)						\
  value_t type##_set(value_t x, int n, eltype v )			\
  {									\
    if ( n < 0 )							\
      n += get( type, x, len );						\
    assert( n > 0 && n < get( type, x, len ) );				\
    ((eltype*)get( type, x, data ))[n] = v;				\
    return x;								\
  }

#define array_resize(type, eltype, strp)		\
  value_t type##_resize( value_t x, int n )		\
  {							\
    static const int _e = sizeof(eltype);		\
    int o   = get( type, x, cap );			\
    n      += strp;					\
    int p   = arr_resize( n );				\
    x       = reallocate( x, o*_e, p*_e );		\
    get( type, x, len )  = n;				\
    get( type, x, cap )  = p;				\
    							\
    return x;						\
  }

#define array_put(type, eltype)					\
  value_t type##_put( value_t x, eltype v )			\
  {								\
    ((eltype*)get( type, x, data ))[get(type, x, len)++] = v;	\
    if ( get( type, x, len ) == get( type, x, cap ) )		\
      x = type##_resize( x, get( type, x, len ) + 1 );		\
    return x;							\
  }

#endif


