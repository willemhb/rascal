#ifndef rascal_template_str_h
#define rascal_template_str_h


#define DECL_STRING(T)				\
  typedef struct T##_t T##_t

#define STRING_TYPE(T, H, V, F, fname)		\
  struct T##_t					\
  {						\
    H						\
    STRING_SLOTS(V, F, fname);			\
  }

#define STRING_SLOTS(V, F, fname)		\
  V       *data;				\
  arity_t  len;					\
  F        fname;				\
  hash_t   hash

#define DECL_STRING_API(T, V, F, fname)					\
  T##_t *new_##T(arity_t n, V *i, F fname);				\
  void   init_##T(T##_t* s, arity_t n, V *i, F fname);			\
  size_t pad_##T##_size(arity_t oldl, arity_t newl, arity_t oldc);	\
  void   finalize_##T(obj_t *s)

#define PAD_TEXT_SIZE(T)						\
  size_t pad_##T##_size(arity_t oldl, arity_t newl, arity_t oldc)	\
  {									\
    (void)oldl;								\
    (void)oldc;								\
    return newl + 1;							\
  }

#define PAD_BUFFER_SIZE(T, V)						\
  size_t pad_##T##_size(arity_t oldl, arity_t newl, arity_t oldc)	\
  {									\
    return pad_stack_size(oldl+1, newl+1, oldc, T##_min_cap);		\
  }

#define NEW_STRING(T, V, F, fname)					\
  T##_t *new_##T(arity_t n, V *i, F fname)				\
  {									\
    if (n == 0)								\
      return empty_##T;							\
    T##_t *out = alloc( sizeof(T##_t) );				\
    init_##T( out, n, i, enc );						\
    return out;								\
  }

#define INIT_STRING(T, V)						\
  void init_##T( T##_t *str, arity_t n, V *i, encoding_t enc)		\
  {									\
    init_obj((obj_t*)str, T##_type, enc);				\
    str->len  = n;							\
    str->cap  = pad_##T##_size( 0, n, 0 );				\
    str->data = alloc( str->cap * sizeof_enc( enc ) );			\
    if (i)								\
      copy( str->data, i, n * sizeof_enc( enc ) );			\
    str->hash = 0;							\
  }

#define STRING_ENCODING(T)			\
  encoding_t get_##T##_enc( T##_t* str )	\
  {						\
    return ((obj_t*)str)->flags & enc_mask;	\
  }

#define FINALIZE_STRING(T, V)						\
  void finalize_##T( obj_t *obj )					\
  {									\
    T##_t* str = (T##_t*)obj;						\
    dealloc( str->data,							\
	     str->cap * sizeof_enc( get_##T##_enc( str ) ) );		\
  }

#endif
