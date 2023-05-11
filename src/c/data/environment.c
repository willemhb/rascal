#include "data/environment.h"

#include "data/table.h"
#include "data/unit.h"


// API & utilities
bool define(value_t name, environment_t* environment) {
  return table_add(environment->locals, name, UNBOUND);
}
