#ifndef rascal_describe_utils_h
#define rascal_describe_utils_h

#define asa( type, x ) ((type##_t*)asptr(x))

#define get( type, x, field ) (asa(type, x)->field)

#define mk_tag_p(t, n) inline bool n##p( value_t x ) { return tag( x ) == type_##t; }
#define mk_val_p(v, n) inline bool n##p( value_t x ) { return x == v; }

#define mk_safe_cast(type, ctype, cnvt)		\
  ctype to##type( char *fname, value_t x )	\
  {						\
    require( type##p( x ),			\
	     fname,				\
	     x,					\
	     "expected a %s, got ",		\
	     #type );				\
    return (ctype)cnvt( x );			\
  }

#define for_cons(c, x)					\
  for (;consp(*c) && ((x=car(*c))||1); *c = cdr(*c))

#define for_vec(v, i, x)				\
  for (i=0;i<alen(*v) && ((x=vdata(*v)[(i)])||1); i++)

#define for_bytes(b, i, x)				\
  for (i=0; i<alen(*b) && ((x=bdata(*b)[(i)])||1); i++)

#define noop_constructor( type )		\
  r_argc( #type, 1, argx );			\
  r_argt( #type, Stack[Sp-1], type_##type );	\
  goto do_fetch

#endif


