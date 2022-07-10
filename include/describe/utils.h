#ifndef rascal_describe_utils_h
#define rascal_describe_utils_h

#define object_type(eltype)				\
  {							\
    union {						\
      eltype *data;					\
      eltype  i_data[8/sizeof(eltype)];			\
    };							\
    value_t type  :  8;					\
    value_t size  : 48; /* */				\
    value_t flags :  8;					\
  }

#define collection_type(eltype)				\
  {							\
    union {						\
      eltype *data;					\
      eltype  i_data[8/sizeof(eltype)];			\
    };							\
    value_t type  :  8;					\
    value_t size  : 48; /* */				\
    value_t flags :  8;					\
    long len, cap;					\
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
    type##_t *a = asptr( x );			\
    if ( n < 0 )				\
      n += a->len;				\
    assert( n > 0 && n < a->len );		\
    return a->data[n];				\
  }

#define array_set(type, eltype)						\
  value_t type##_set(value_t x, int n, eltype v )			\
  {									\
    type##_t *a = asptr( x );						\
    if ( n < 0 )							\
      n += a->len;							\
    assert( n > 0 && n < a->len );					\
    a->data[n] = v;							\
    return x;								\
  }

#define array_resize(type, eltype, strp)				\
  type##_t *type##_resize( type##_t* a, int n )				\
  {									\
    static const int _e = sizeof(eltype);				\
    int o = a->cap;							\
    int p = arr_resize( n + strp );					\
    a = (type##_t*)reallocate( (object_t*)a, p*_e, o*_e );		\
    a->len = n;								\
    a->cap = p;								\
    return a;								\
  }

#define array_put(type, eltype, strp)				\
  value_t type##_put( value_t x, eltype v )			\
  {								\
    type##_t* a = asptr( x );					\
    a->data[a->len++] = v;					\
    if ( a->len + strp == a->cap )				\
      a = type##_resize(  a, a->len+strp+1 );			\
    return x;							\
  }

#endif


