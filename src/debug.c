#include <stdio.h>
#include <string.h>

#include "debug.h"

void displayValues(FILE* ios, Values* values) {
  fprintf(ios,
          "Contents of values (count=%zu) (capacity=%zu): \n",
          values->count,
          values->capacity);

  for (size_t i=0; i<values->count; i++) {
    fprintf(ios, "    ");
    printValue(ios, values->data[i]);
    fprintf(ios, "\n");
  }
}
