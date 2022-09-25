#ifndef rascal_template_str_h
#define rascal_template_str_h

#define DECL_STRING(T, V)			\
  typedef struct T##_init_t			\
  {						\
    init_t init;				\
    bool has_hash;				\
    hash_t hash;				\
  } T##_init_t;					\
  						\
  struct T##_t					\
  {						\
    OBJ_HEAD;					\
    V         *vals;				\
    arity_t    len;				\
    encoding_t enc;				\
    hash_t     hash;				\
  }

#define DECL_BUFFER(T, V)			\
  struct T##_t					\
  {						\
    OBJ_HEAD;					\
    V          *vals;				\
    arity_t     len;				\
    arity_t     cap;				\
    encoding_t  enc;				\
  }

#define PAD_TEXT_SIZE(T, V)						\
  size_t pad_##T##_size(size_t oldl, size_t newl, size_t oldc)		\
    {									\
      (void)oldl;							\
      (void)oldc;							\
      return newl + 1;							\
    }

#define PAD_BUFFER_SIZE(T, V)					\
  size_t pad_##T##_size(size_t oldl, size_t newl, size_t oldc)	\
  {								\
    return pad_stack_size(oldl+1, newl+1, oldc, T##_min_cap);	\
  }

#define NEW_STRING(T, V)						\
  obj_t *new_##T(init_t *init)						\
  {									\
    obj_t *out;								\
    flags_t fl = init->flags;						\
    arity_t n = init->n;						\
    if (n == 0)								\
      {									\
	out = FamousObjects[init->type];				\
	init->fl |= INIT_NONE;						\
      }									\
    else if (flag_p(fl, INIT_PARENT))					\
      {									\
	out = init->obj;						\
	init->fl |= INIT_NONE;						\
      }									\
    else if (flag_p(fl, STATIC_OBJ))					\
      out = init->obj;							\
    else								\
      out = alloc( repr->base_size );					\
    if (!flag_p(fl, INIT_NONE))						\
      {									\
	arity_t cap = pad_##T##_size(0, n, 0);				\
	((T##_t*)out)->len = n;						\
	if (flag_p(fl, STATIC_DATA))					\
	  ((T##_t*)out)->vals = init->data;				\
	else								\
	  ((T##_t*)out)->vals = allocv(cap, V);				\
      }									\
    return out;								\
  }

#define INIT_STRING(T, V, hash_fn)					\
  void init_##T( obj_t *obj, init_t *init )				\
  {									\
    T##_t *string = (T##_t*)obj;					\
    T##_init_t *str_init = (T##_init_t*)init;				\
    repr_t *repr = Reprs[init->type];					\
    string->encoding = (encoding_t)repr->el_Ctype;			\
    									\
    if (flag_p(f, INIT_PARENT))						\
      {									\
	T##_t *parent = init->data;					\
	memcpy( string->vals, parent->vals, init->n * sizeof(V));	\
      }									\
    else if (flag_p(f, INIT_SPECIAL))					\
      {									\
	memcpy( string->vals, init->data, init->n * sizeof(V));		\
      }									\
    else if (flag_p(f, INIT_STACK))					\
      {									\
	val_t *vals = (val_t*)init->data;				\
	V     *spc  = string->vals;					\
	for (arity_t i=0; i<a; i++, spc++)				\
	  {								\
	    int result = repr->do_init_vals(spc, vals[i]);		\
	    if (result)							\
	      type_error(((str_t*)repr->name)->vals,			\
			 vals[i],					\
			 repr->el_type );				\
	  }								\
      }									\
    else								\
      {									\
      }									\
    if (str_init->has_hash)						\
      {									\
	string->hash = str_init->hash;					\
      }									\
    else								\
      {									\
	string->hash = hash_fn( string->vals );				\
      }									\
  }

#endif
