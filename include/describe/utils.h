#ifndef rascal_describe_utils_h
#define rascal_describe_utils_h

#define collection_type(eltype)			\
  {						\
    value_t type;				\
    size_t len, cap;				\
    eltype *data;				\
  }

#define mk_tag_p(t) inline bool t##p( value_t x ) { return x && vtag(x) == tag_##t; }

#define mk_type_p(t) inline bool t##p( value_t x ) { return val_typeof( x ) == type_##t; }

#define mk_type_dispatch(rtype, fname, tableName)			\
  inline rtype val_##fname( value_t x )					\
  {									\
    return tableName[val_typeof(x)];					\
  }

#define array_ref(type, eltype)			\
  eltype type##_ref(object_t *x, int n )	\
  {						\
    if ( n < 0 )				\
      n += x->len;				\
    assert( n > 0 && (size_t)n < x->len );	\
    return x->data[n];				\
  }

#define array_set(type, eltype)				\
  object_t *type##_set(object_t *x, int n, eltype v )	\
  {							\
    if ( n < 0 )					\
      n += x->len;					\
    assert( n > 0 && (size_t)n < x->len );		\
    ((eltype*)x->data)[n] = v;				\
    return x;						\
  }

#define array_resize(type, eltype, strp)		\
  object_t *type##_resize( object_t *x, int n )		\
  {							\
    int o   = x->cap;					\
    n      += strp;					\
    int p   = arr_resize( n );				\
    x       = reallocate( x, o*eltype, p*eltype );	\
    x->len  = n;					\
    x->cap  = p;					\
    							\
    return x;						\
  }

#define array_put(type, eltype)				\
  object_t *type##_put( object_t *x, eltype v )		\
  {							\
    ((eltype*)x->data)[x->len++] = v;			\
    if (x->len == x->cap)				\
      x = array_resize( x, x->len+1 );			\
    return x;						\
  }

#endif


