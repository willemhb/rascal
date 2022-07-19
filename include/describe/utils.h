#ifndef rascal_describe_utils_h
#define rascal_describe_utils_h


#define get( type, x, field ) (as##type(x)->field)
#define get_s( fname, type, x, field ) (to##type( fname, x )->field)

#define mk_tag_p(t) bool is_##t( value_t x ) { return tag( x ) == tag_##t; }
#define mk_val_p(v) bool is_##v( value_t x ) { return x == val_##v; }
#define mk_type_p(t) bool is_##t( value_t x ) { return r_type( x ) == type_##t; }

#define r_builtin(name) builtin_##name(size_t n)

#define mk_builtin(name)    builtin( #name, builtin_##name )
#define mk_builtin_p(name)  builtin( #name"?", builtin_is_##name )

#define mk_safe_cast(type, ctype, cnvt, is_ob)		\
  ctype to##type( const char *fname, value_t x )	\
  {							\
    require( fname,					\
	     is_##type( x ),				\
	     "# wanted a %s()",				\
	     #type );					\
    							\
    if ( is_ob )					\
      require( fname,					\
	       !is_immediate(x),			\
	       "# wanted a non-empty %s()",		\
	       #type);					\
    							\
    return (ctype)cnvt( x );				\
  }

#define for_cons(c, x)					\
  for (;is_cons(*c) && ((x=car(*c))||1); *c = cdr(*c))

#define unpack_cons(c, n)			\
  do						\
    {						\
      while (is_cons(*c)) 			\
	{					\
	  push(car(*c));			\
	  *c = cdr(*c);				\
	  n++;					\
	}					\
    } while (0)

#define unpack_vector(v, n)			\
  do						\
    {						\
      for (size_t _i=0; _i<alength(v); _i++)	\
	{					\
	  push( elements(v)[_i] );		\
	  n++;					\
	}					\
    } while (0)

#define unpack_binary(b, n)			\
  do						\
    {						\
      for (size_t _i=0; _i<alength(b); _i++)	\
	{					\
	  fixnum_t _f = binary_get( b, _i );	\
	  push( fixnum( _f ) );			\
	  n++;					\
	}					\
    } while (0)

#define unpack_atom(a, n)			\
  do {						\
    push(a);					\
    n++;					\
  } while (0)


#define for_vec(v, i, x)				\
  for (i=0;i<alen(*v) && ((x=vdata(*v)[(i)])||1); i++)


#define for_bytes(b, i, x)				\
  for (i=0; i<alen(*b) && ((x=bdata(*b)[(i)])||1); i++)

#define r_predicate(fname)			\
  void r_aligned builtin_is_##fname( size_t n )	\
  {						\
  argc( #fname"?", n, 1);			\
  Sref(1) = boolean( is_##fname( Sref(1) ) );	\
  }

#define r_getter(sname, obtype)				\
  void r_aligned builtin_##sname( size_t n )		\
  {							\
    argc( #sname, n, 1 );				\
    Sref(1) = to##obtype( #sname, Sref(1) )->sname;	\
  }

#define r_setter(sname, fname, obtype)					\
  void r_aligned builtin_##fname( size_t n )				\
  {									\
    argc( #fname, n, 2 );						\
    to##obtype( #fname, Sref(2) )->sname = Sref(1);			\
    Stack[Sp-2] = Stack[Sp-1]; Sp--;					\
  }

#define r_arithmetic(name, rname, op, divp, argcMin, uarg, s)		\
  void builtin_##name( size_t n )					\
  {									\
    vargc( rname, n, argcMin );						\
    fixnum_t acc = tofixnum( rname, Sref(1) );				\
    if (argcMin == 1 && n == 1) {					\
      acc = uarg( acc );						\
    } else {								\
      for (size_t i=2; i<=n && (s > -1 && acc != s); i++) {		\
	fixnum_t tmp = tofixnum( rname, Sref(i) );			\
	if (divp && tmp == 0)						\
	  error( rname,							\
		 "zero-divison in (%s %dl 0)",				\
		 #rname,						\
		 acc );							\
	  else 								\
	    acc = acc op tmp;						\
      }									\
    }									\
    popn( n );								\
    push( fixnum( acc ) );						\
  }

#define r_arithmetic_p( name, rname, op )		\
  void builtin_##name( size_t n )			\
  {							\
    vargc( rname, n, 2 );				\
    fixnum_t x = tofixnum( rname, Sref(1) );		\
    bool sentinel = true;				\
    for (size_t i=2; sentinel && i<=n; i++)		\
      {							\
	fixnum_t y = tofixnum( rname, Sref(i) );	\
	sentinel   = (x op y);				\
	x = y;						\
      }							\
    Sref(n) = boolean( sentinel );			\
    Sp -= n-1;						\
  }

#define r_aligned __attribute__((aligned(8)))

#endif


