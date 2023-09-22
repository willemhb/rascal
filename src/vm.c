#include "vm.h"

// global variables
#include "tpl/describe.h"

bool compareGlobalEnvKeys(Atom* x, Atom* y) {
  return x == y;
}

void internGlobalEnvKey(GlobalEnvEntry* entry, Atom* key, Value* value) {
  entry->key = key;
  entry->val = *value == NOTHING_VAL ? NIL_VAL : *value;
}

TABLE_TYPE(GlobalEnv,
           globalEnv,
           Atom*,
           Value,
           compareGlobalEnvKeys,
           hashObject,
           internGlobalEnvKey,
           NULL,
           NOTHING_VAL);

