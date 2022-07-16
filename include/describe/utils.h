#ifndef rascal_describe_utils_h
#define rascal_describe_utils_h


#define get( type, x, field ) (as##type(x)->field)

#define get_s( fname, type, x, field ) (to##type( fname, x )->field)


#define mk_tag_p(t) bool t##p( value_t x ) { return gettag( x ) == tag_##t; }
#define mk_val_p(v) bool v##p( value_t x ) { return x == val_##v; }
#define mk_type_p(t) bool t##p( value_t x ) { return r_type( x ) == type_##t; }

#define r_builtin(name) __attribute__((aligned(8))) r_##name( int n )


#define array_type(eltype)				\
  object_t base;					\
  eltype data[0]

#define mk_safe_cast(type, ctype, cnvt)		\
  ctype to##type( char *fname, value_t x )	\
  {						\
    require( fname,				\
	     type##p( x ),			\
	     "# wanted a %s()",			\
	     #type );				\
    return (ctype)cnvt( x );			\
  }

#define for_cons(c, x)					\
  for (;consp(*c) && ((x=car(*c))||1); *c = cdr(*c))

#define for_vec(v, i, x)				\
  for (i=0;i<alen(*v) && ((x=vdata(*v)[(i)])||1); i++)

#define for_bytes(b, i, x)				\
  for (i=0; i<alen(*b) && ((x=bdata(*b)[(i)])||1); i++)

#define r_predicate(fname)			\
  void r_aligned r_##fname##p( size_t n )	\
  {						\
  argc( #fname"?", n, 1);			\
  Tos = mk_bool( fname##p( Tos ) );		\
  }

#define r_getter(sname, obtype)			\
  void r_aligned r_##sname( size_t n )		\
  {						\
    argc( #sname, n, 1 );			\
    Tos = to##obtype( #sname, Tos )->sname;	\
  }

#define r_setter(sname, fname, obtype)					\
  void r_aligned r_##fname( size_t n )					\
  {									\
    argc( #fname, n, 2 );						\
    to##obtype( #fname, Peek( 2 ) )->sname = Tos;			\
    rotate();								\
    pop();								\
  }

#define r_arithmetic(name, rname, op, divp, argcMin, uarg, s)		\
  void r_aligned r_##name( size_t n )					\
  {									\
    vargc( rname, n, argcMin );						\
    fixnum_t acc = tofixnum( rname, Arg(1) );				\
    if (argcMin == 1 && n == 1) {					\
      acc = uarg( acc );						\
    } else {								\
      for (size_t i=2; i<=n && (s > -1 && acc != s); i++) {		\
	fixnum_t tmp = tofixnum( rname, Arg(i) );			\
	if (divp && tmp == 0)						\
	  r_error( rname, "zero-divison" );				\
	acc = acc op tmp;						\
      }									\
    }									\
    Tos = mk_fixnum( acc );						\
    swopn( n );								\
  }

#define r_arithmeticp( name, rname, op )		\
  void r_aligned r_##name( size_t n )			\
  {							\
    vargc( rname, n, 2 );				\
    fixnum_t x = tofixnum( rname, Arg(1) );		\
    bool sentinel = true;				\
    for (size_t i=2; sentinel && i<=n; i++)		\
      {							\
	fixnum_t y = tofixnum( rname, Arg(i) );		\
	sentinel   = (x op y);				\
	x = y;						\
      }							\
    Tos = mk_bool( sentinel );				\
    swopn( n );						\
  }							\

#define r_aligned __attribute__((aligned(8)))

#endif


