#ifndef rascal_str_h
#define rascal_str_h

#include <uchar.h>

#include "obj.h"
#include "mem.h"
#include "utils/str.h"
#include "template/str.h"

typedef struct str_t str_t;

typedef enum
  {
    str_fl_hashed=0x0100, // whether the hash has been computed
  } str_fl_t;

DECL_STRING(str, char_t);
DECL_STRING_API(str, char_t);

// globals
extern str_t *empty_str;

// initialization
void str_init( void );

// convenience
#define is_str(val) (is_obj_type(val, str_type))
#define as_str(val) ((str_t*)as_ptr(val))



#endif
