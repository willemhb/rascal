#ifndef rascal_obj_memory_h
#define rascal_obj_memory_h

#include "obj/array.h"

typedef struct heap_t      heap_t;
typedef struct objects_t   objects_t;
typedef struct behaviors_t behaviors_t;

typedef void (*behavior_t)( void );

// heap type
struct heap_t
{
  OBJECT

  heap_object_t *live_objects;

  object_t      *grays;
  object_t      *initializers;
  object_t      *marks;
  object_t      *unmarks;
  object_t      *finalizers;

  size_t         allocated;
  size_t         alloccap;

  bool           read_barrier;
  bool           write_barrier;
};

struct objects_t
{
  ARRAY(object_t*);
};

struct behaviors_t
{
  ARRAY(behavior_t);
};

// globals
extern type_t HeapType, ObjectsType, BehaviorsType;

// forward declarations
void heap_init( heap_t *heap );
void heap_free( heap_t *heap );
void push_gray( heap_t *heap, object_t *obj );
void register_init( heap_t *heap, behavior_t behavior );
void register_mark( heap_t *heap, behavior_t behavior );
void register_unmark( heap_t *heap, behavior_t behavior );
void register_finalize( heap_t *heap, behavior_t behavior );

void rl_obj_memory_init( void );
void rl_obj_memory_finalize( void );

// convenience


#endif
