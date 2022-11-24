#ifndef rl_vm_heap_h
#define rl_vm_heap_h

#include "rascal.h"

/* C types */
typedef struct heap_t
{
  vector_t *gray_objects;
  object_t *live_objects;
  size_t    bytes_allocated;
  size_t    next_cycle;
} heap_t;

/* globals */
extern heap_t Heap;

/* API */

/* initialization */
void rl_vm_heap_init( void );

/* utilities & convenience */

#endif
