#ifndef rascal_record_h
#define rascal_record_h

#include "obj/object.h"

struct record_t
{
  boxed_t   boxed;
  byte      data[0];
};


#endif
