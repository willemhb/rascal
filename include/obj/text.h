#ifndef rl_obj_text_h
#define rl_obj_text_h

#include "rascal.h"

#include "vm/obj.h"

#include "vm/obj/support/string.h"


/* commentary 

   glyph and string types */

/* C types */

struct str_t {
  OBJ;

  string_t string;
};


/* globals */
extern str_t EmptyStr;

/* API */
val_t string( string_t chars );

/* runtime dispatch */
void rl_obj_text_init( void );
void rl_obj_text_mark( void );
void rl_obj_text_cleanup( void );

/* convenience */

#endif
