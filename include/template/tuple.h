#ifndef rascal_tuple_template_h
#define rascal_tuple_template_h

#define DECL_SIG(T, V, F)				\
  typedef struct T##_t T##_t;				\
  struct T##_t						\
  {							\
    V       *data;					\
    arity_t  len;					\
    V        F;						\
  }

#define DECL_SIG_API(T, V, F)						\
  T##_t *new_##T(V F, arity_t n, V *ini);				\
  void   init_##T(T##_t* s, V F, arity_t n, V *i);			\
  T##_t *mk_##T(V F, arity_t n, ...);					\
  void   finalize_##T(T##_t *s);					\
  int    validate_##T(T##_t *s, val_t *args, arity_t n)


#define NEW_SIG(T, V, F)			\
  T##_t *new_##T(V F, arity_t n, V *ini)	\
  {						\
    T##_t *out = alloc( sizeof(T##_t) );	\
    init_##T( out, F, n, ini );			\
    return out;					\
  }

#define INIT_SIG(T, V, F)				\
  void init_##T(T##_t *sig, V F, arity_t n, V *ini)	\
  {							\
    sig->len  = n;					\
    sig->F    = F;					\
    sig->data = allocv( n, V );				\
    if (ini)						\
      copyv( sig->data, ini, n, V );			\
  }

#define MK_SIG(T, V, F)				\
  T##_t *mk_##T( V F, arity_t n, ... )		\
  {						\
    V buffer[n];				\
    va_list va;					\
    va_start(va,n);				\
    for (arity_t i=0; i<n; i++)			\
      buffer[i] = va_arg(va,V);			\
    va_end(va);					\
    return new_##T(F, n, buffer);		\
  }

#define FINALIZE_SIG(T, V, F)			\
  void finalize_##T(T##_t *s)			\
  {						\
    deallocv( s->data, s->len, V );		\
  }


#endif
