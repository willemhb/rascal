#ifndef error_h
#define error_h

#include <stdarg.h>

#include "value.h"

/* C types */
typedef enum ErrorType ErrorType;

enum ErrorType {
  NO_ERROR,
  READ_ERROR,
  COMP_ERROR,
  EVAL_ERROR,
  APPLY_ERROR
};

#define ERROR_BUFFER_SIZE 2048

struct Error {
  bool      panicking;
  ErrorType type;
  Val       irritant;
  char      message[2048];
};

/* globals */
extern struct Error Error;

/* API */
#define GUARD(_condition, _fname, _type, _irritant, _fmt, ...)	\
  do {								\
    if (!panicking() && (_condition)) {				\
      panic((_fname),						\
	    (_type),						\
	    (_irritant),					\
	    (_fmt) __VA_OPT__(,) __VA_ARGS__);			\
    }								\
  } while (false)

#define APPLY_GUARD(_condition, _fname, _irritant, _fmt, ...)		\
  GUARD((_condition),							\
	(_fname),							\
	APPLY_ERROR,							\
	(_irritant),							\
	(_fmt) __VA_OPT__(,) __VA_ARGS__)

#define ARITY_GUARD(_fname, _argc, _n_args)			\
  APPLY_GUARD((_argc) == (_n_args),				\
	      (_fname),						\
	      mk_small(_n_args),				\
	      "expected %d arguments",				\
	      (_argc))

#define VARITY_GUARD(_fname, _argc, _n_args)		\
  APPLY_GUARD((_n_args) >= (_argc),			\
	      (_fname),					\
	      mk_small(_n_args),			\
	      "expected at least %d arguments",		\
	      (_argc))

bool panicking(void);
void panic(const char *fname, ErrorType type, Val irritant, const char *fmt, ...);
void vpanic(const char *fname, ErrorType type, Val irritant, const char *fmt, va_list va);
ErrorType recover(void);

#endif
