#ifndef compare_h
#define compare_h

#include "common.h"

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
uhash hash_object( void* ptr, bool id );
uhash hash_value( value_t x, bool id );
int compare_objects( void* px, void* py, bool id, bool eq );
int compare_values( value_t x, value_t y, bool id, bool eq );

#endif
