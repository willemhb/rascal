#ifndef rl_val_text_h
#define rl_val_text_h

#include "val/object.h"

/* Declarations & APIs for text types. */

/* C types */
struct String {
  HEADER;
  char*  data;
  size_t cnt;
};

struct Buffer8 {
  HEADER;
  char*  data;
  size_t cnt;
  size_t cap;
};

/* Globals */
extern struct Type StringType, Buffer8Type, GlyphType;

/* External APIs */

/* initialization */

#endif
