#ifndef data_namespace_h
#define data_namespace_h

#include "data/object.h"

struct namespace {
  HEADER;
  namespace_t* parent;
  table_t*     locals;
  table_t*     upvalues;
};

typedef enum {
  LOCALNS    = 0x01,
  SCRIPTNS   = 0x02,
  TOPLEVELNS = 0x03
} nmspcfl_t;

// APIs & utilities

#endif
