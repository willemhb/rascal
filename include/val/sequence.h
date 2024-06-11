#ifndef rl_val_sequence_h
#define rl_val_sequence_h

#include "val/object.h"

/* Types, APIs, and globals for sequence interface objects. */

/* C types */
struct Sequence {
  HEADER;

  // bit fields
  word_t transient : 1;
  word_t firsted   : 1;
  word_t finished  : 1;

  // data fields
  Sequence* parent;    // for inductive data structures, this holds the state for the parent object
  Object*   source;    // the object being iterated over
  Value     first;     // cached call to (first s)
  Value     state;     // state key (probably an offset)
};

/* Globals */
extern Type SequenceType;

/* APIs */
void* rl_seq(void* x, bool t);
bool  rl_donep(void* x);
void* rl_first(void* x);
void* rl_rest(void* x);

#endif
