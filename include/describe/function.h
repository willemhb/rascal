#ifndef describe_function_h
#define describe_function_h


#define DeclareBuiltinFunction(_name) value_t rsp_##_name(value_t args, size_t nargs)
#define DeclareBuiltinEnsure(_name) bool_t   ensure_##_name(value_t args, size_t nargs)
#define DeclareBuiltin(_name)			        \
  DeclareBuiltinFunction(_name);		        \
  DeclareBuiltinEnsure(_name)

#define EnsureFargBuiltin(_name, _nargs, ...)		\
  int_t ensure_##_name(value_t *args, size_t nargs)	\
  {							\
    static bool_t (*validators[_nargs])(value_t x) = 	\
      {							\
	##__VA_ARGS__					\
      };						\
    							\
    if (nargs != _nargs)				\
      return -1;					\
    for (size_t i=0; i < _nargs; i++)			\
      {							\
	bool_t (*validator)(value_t x) = validators[i];	\
	if (!validator || validator(args[i])) continue;	\
	return i;					\
      }							\
    return 0;						\
  }

#define EnsureVargBuiltin(_name, _nargs, ...)           \
  int_t ensure_##_name(value_t *args, size_t nargs)	\
  {							\
    static bool_t (*validators[_nargs+1])(value_t x) = 	\
      {							\
	##__VA_ARGS__					\
      };						\
    if (nargs < _nargs)					\
      return -1;					\					\
    for (size_t i=0; i < _nargs; i++)			\
      {							\
	bool_t (*validator)(value_t x) = validators[i];	\
	if (!validator || validator(args[i])) continue;	\
	return i;					\
      }							\
    bool_t (*validator)(value_t x) = validators[i];	\
    if (!validator)					\
      return 0;						\
    for (size_t i=_nargs; i<nargs; i++)			\
      {							\
	if (validator(args[i]))	continue;		\
	return 0;					\
      }							\
    return 0;						\
  }

#define EnsureBuiltin(_name, _nopts, ...)				\
  int_t ensure_##_name(value_t *args, size_t nargs)			\
  {									\
    static int_t (*validators[_nopts])(value_t *a, size_t n)		\
    {									\
      ##__VA_ARGS__							\
    };									\
    for (size_t i=0; i < _nopts;)					\
      {									\
	int_t (*validator)(value_t *a, size_t n) = validators[i];	\
	int_t result = validator(args, nargs);				\
	if (result > 0) continue;					\
	else if (result < 0) i++;					\
	else return 0;							\
      }									\
    return -2;								\
  }

#define DescribeArithmetic( _name, _op, _sentinel  )			\
  value_t rsp_##_name( value_t *args, size_t nargs )			\
  {									\
    /* the code checking _sentinel should be compiled away		\
       wherever it's not relevant, ie, the check first check		\
       should be eliminated except in the case of subtraction.		\
     */									\
    									\
    long_t buf = ival(args[0]);						\
    if (_sentinel == -1 && nargs == 1)					\
      buf *= -1;							\
    else								\
      for (size_t i=1; _sentinel > 0 && i != _sentinel && i<nargs; i++)	\
	{								\
	  long_t next = ival(args[i]);					\
	  buf = buf _op next;						\
	}								\
    return integer(buf);						\
  }

#define DescribeArithmeticPredicate(_name, _op )                        \
  value_t rsp_##_name( value_t *args, size_t nargs )			\
  {									\
    bool_t result = true;						\
    long_t buf    = ival(args[0]);					\
    for (size_t i=1; result && i <nargs; i++)				\
      {									\
	long_t next = ival(args[i]);					\
	result      = buf _op next;					\
	buf         = next;						\
      }									\
    return result ? rtrue : rfalse;					\
  }


#endif
