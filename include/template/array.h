#ifndef rascal_template_array_h
#define rascal_template_array_h

// template for dynamic array types
#define DECL_ALIST(T, V)						\
  struct T##_t								\
  {									\
    OBJ_HEAD;								\
    V      *vals;							\
    arity_t len;							\
    arity_t cap;							\
  }

#define PAD_ALIST_SIZE(T, V)						\
  size_t pad_##T##_size(size_t oldl, size_t newl, size_t oldc)		\
  {									\
    return pad_alist_size(oldl, newl, oldc);				\
  }

#define PAD_STACK_SIZE(T, V)						\
  size_t pad_##T##_size(size_t oldl, size_t newl, size_t oldc)		\
  {									\
    return pad_stack_size(oldl, newl, oldc, T##_min_cap);		\
  }

#define RESIZE_ARRAY(T, V)						\
  obj_t *resize_##T(obj_t* obj, arity_t newl)				\
  {									\
    if (is_famous(obj))							\
      return new( obj->type, 0, newl, NULL );				\
    T##_t * array = (T##_t*)obj;					\
    size_t oldc = array->cap;						\
    size_t newc = pad_##T##_size( array->len, newl, oldc );		\
    if (oldc != newc)							\
      array->vals = resizev( array->vals, oldc, newc, V );		\
    return obj;								\
  }

#define INIT_ARRAY(T, V)						\
  void init_##T(obj_t *o, init_t *init)					\
  {									\
    T##_t      *array  = (T##_t*)o;					\
    arity_t n = init->n, cap = pad_##T##_size( 0, n, 0 );		\
    array->len         = n;						\
    array->cap         = cap;						\
    array->vals        = allocv(cap, V);				\
    									\
    if (flag_p(f, INIT_PARENT))						\
      {									\
	T##_t *parent = (T##_t*)i;					\
	memcpy( array->vals, parent->vals, a * sizeof(V) );		\
      }									\
    else if (flag_p(f, INIT_SPECIAL))					\
      {									\
	memcpy( array->vals, (V*)i, a * sizeof(V) );			\
      }									\
    else if (flag_p(f, INIT_STACK))					\
      {									\
	repr_t *repr = Reprs[t];					\
	val_t  *vals = (val_t*)i;					\
	V      *spc  = array->vals;					\
									\
	if (repr->do_init_vals)						\
	  {								\
	    for (arity_t i=0; i<a;i++, spc++)				\
	      {								\
		int result = repr->do_init_vals(spc, vals[i]);		\
		if (result)						\
		  {							\
		    type_error(((str_t*)repr->name)->vals,		\
			       vals[i],					\
			       repr->el_type );				\
		  }							\
	      }								\
	  }								\
	else								\
	  {								\
	    memcpy( spc, vals, a * sizeof(V));				\
	  }								\
      }									\
  }

#define TRACE_ARRAY(T, V)			\
  void trace_##T(obj_t *obj)			\
  {						\
    T##_t *array = (T##_t*)obj;			\
    trace(array->vals, array->len);		\
  }

#define FINALIZE_ARRAY(T, V)			\
  void finalize_##T(obj_t *obj)			\
  {						\
    T##_t *array = (T##_t*)obj;			\
    deallocv( array->vals, V );			\
  }


#endif
