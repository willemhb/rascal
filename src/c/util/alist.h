#ifndef util_alist_h
#define util_alist_h

#include "common.h"

// dynamic array of tagged data +++++++++++++++++++++++++++++++++++++++++++++++
struct values {
  value_t* data;
  usize cnt, cap;
};

void init_values( values_t* slf );
void free_values( values_t* slf );
void reset_values( values_t* slf );
usize resize_values( values_t* slf, usize n );
usize values_push( values_t* slf, value_t val );
value_t values_pop( values_t* slf );
void trace_values( values_t* slf );

// dynamic array of objects +++++++++++++++++++++++++++++++++++++++++++++++++++
struct objects {
  object_t** data;
  usize cnt, cap;
};

void init_objects( objects_t* slf );
void free_objects( objects_t* slf );
void reset_objects( objects_t* slf );
usize resize_objects( objects_t* slf, usize n );
usize objects_push( objects_t* slf, void* val );
void* objects_pop( objects_t* slf );
void trace_objects( objects_t* slf );

#endif
