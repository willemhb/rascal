#ifndef rascal_runtime_error_h
#define rascal_runtime_error_h

#include "rascal.h"

#define  _cinfo "%s:%s:%d:"

#define efmt(_fmt) (_cinfo			\
		    _fmt)

// utilities -----------------------------------------------------------------
void rsc_error( value_t agitant, char *fmt, ... );
void rsc_require( bool test, value_t agitant, char *fmt, ... );

#define error(agitant, fmt, ...)					\
  rsc_error(agitant,							\
	    efmt(fmt),							\
	    __FILE__,							\
	    __LINE__,							\
	    __func__,							\
	    ##__VA_ARGS__ )

#define require(test, agitant, fmt, ...)				\
  rs_require(test,							\
	     agitant,							\
	     efmt(fmt),							\
	     __FILE__,							\
	     __LINE__,							\
	     __func__,							\
	     ##__VA_ARGS__ )


#endif
