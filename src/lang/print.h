#ifndef lang_print_h
#define lang_print_h

#include "base/value.h"
#include "base/type.h"

/* C types */
typedef struct Print Print;

struct Print {
  Stream* ios;
  Table*  backrefs;
};

/* API */
void print(Val x);

#endif
