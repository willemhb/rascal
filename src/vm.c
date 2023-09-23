#include "vm.h"

// misc macros
#define N_HEAP (655536*sizeof(Value))

// global variables
Value     theStack[N_STACK];
CallFrame theFrames[N_FRAME];

Vm vm = {
  // heap
  .objects =NULL,
  .heapUsed=0,
  .heapCapacity=N_HEAP,
  .grays={
    .data    =NULL,
    .count   =0,
    .capacity=0
  },

  // interpreter
  .sp=theStack,
  .fp=theFrames,
  .openUpValues=NULL,

  // globals/symbol table
  .symbolCounter=0,
  .symbolTable={
    .table   =NULL,
    .count   =0,
    .capacity=0
  },
  .globalEnv={
    .table   =NULL,
    .count   =0,
    .capacity=0
  },

  // scanner state
  .scanner={
    .start   =NULL,
    .current =NULL,
    .lineNo  =0,
    .hadError=false,
    .tokens  ={
      .data    =NULL,
      .count   =0,
      .capacity=0
    }
  },

  // parser state
  .parser={
    .offset  =0,
    .hadError=false
  }
};

// templates
#include "tpl/describe.h"

bool compareNameSpaceKeys(Symbol* x, Symbol* y) {
  return x == y;
}

void internNameSpaceKey(NameSpaceEntry* entry, Symbol* key, Value* value) {
  entry->key = key;
  entry->val = *value == NOTHING_VAL ? NIL_VAL : *value;
}

TABLE_TYPE(NameSpace,
           nameSpace,
           Symbol*,
           Value,
           compareNameSpaceKeys,
           hashObject,
           internNameSpaceKey,
           NULL,
           NOTHING_VAL);
