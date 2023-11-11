#ifndef rl_val_table_h
#define rl_val_table_h

#include "val/object.h"

/* Mutable hashed collection types. */

/* C types */
typedef enum {
  FASTHASH=0x01,
} TableFl;

typedef struct {
  Value key;
  Value val;
} Entry;

struct Table {
  HEADER;
  Entry* data;
  size_t cnt;
  size_t cap;
};

struct MutSet {
  HEADER;
  Value*  data;
  size_t  cnt;
  size_t  cap;
};

/* globals */
extern struct Type TableType, MutSetType;

/* External APIs */


#endif
