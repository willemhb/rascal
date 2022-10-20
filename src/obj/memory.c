#include <string.h>

#include "obj/memory.h"
#include "obj/symbol.h"
#include "obj/type.h"
#include "vm/memory.h"


static const size_t init_alloccap  = 2097152; // sizeof(cons_t) * (1<<16)
static const size_t objects_minc   = 512;
static const size_t behaviors_minc = 64;

extern heap_t Heap;
extern objects_t   Grays;
extern behaviors_t Inits, Marks, Unmarks, Finalizers;

void heap_init( heap_t *heap )
{
  init_static_object( &heap->obj, &HeapType, true );
  init_static_object( &Grays.obj, &ObjectsType, true );
  init_static_object( &Inits.obj, &BehaviorsType, true );
  init_static_object( &Marks.obj, &BehaviorsType, true );
  init_static_object( &Unmarks.obj, &BehaviorsType, true );
  init_static_object( &Finalizers.obj, &BehaviorsType, true );

  heap->live_objects  = NULL;
  heap->grays         = &Grays.obj;
  heap->initializers  = &Inits.obj;
  heap->marks         = &Marks.obj;
  heap->unmarks       = &Unmarks.obj;
  heap->finalizers    = &Finalizers.obj;
  heap->allocated     = 0;
  heap->alloccap      = init_alloccap;
  heap->read_barrier  = false;
  heap->write_barrier = false;

  init_objects( &Grays.obj, &ObjectsType, 0, NULL );
  init_behaviors( &Inits.obj, &BehaviorsType, 0, NULL );
  init_behaviors( &Marks.obj, &BehaviorsType, 0, NULL );
  init_behaviors( &Unmarks.obj, &BehaviorsType, 0, NULL );
  init_behaviors( &Finalizers.obj, &BehaviorsType, 0, NULL );

  register_init( heap, rl_obj_symbol_init );
  register_init( heap, rl_obj_type_init );
  register_init( heap, rl_obj_memory_init );
}

void heap_free( heap_t *heap )
{
  free_objects( heap->grays );
  free_behaviors( heap->initializers );
  free_behaviors( heap->marks );
  free_behaviors( heap->unmarks );
  free_behaviors( heap->finalizers );

  heap_object_t *live = heap->live_objects;

  while(live != NULL)
    {
      heap_object_t *tmp = live->next;

      obj_free( &live->obj );
      live = tmp;
    }
}

void push_gray( heap_t *heap, object_t *obj )
{
  objects_push( heap->grays, obj );
}

// objects implementation
ARRAY_INIT( objects, object_t*, pad_stack_size, objects_minc );
ARRAY_FREE( objects, object_t* );
ARRAY_RESIZE( objects, object_t*, pad_stack_size, objects_minc );
ARRAY_TRACE( objects, object_t* );
ARRAY_PUSH( objects, object_t* );
ARRAY_POP( objects, object_t* );
ARRAY_POP_S( objects, object_t* );

ARRAY_INIT( behaviors, behavior_t, pad_alist_size, behaviors_minc );
ARRAY_FREE( behaviors, behavior_t );
ARRAY_RESIZE( behaviors, behavior_t, pad_alist_size, behaviors_minc );
ARRAY_PUSH( behaviors, behavior_t );

// globals
layout8_t HeapLayout =
  {
    { .base_size = sizeof(heap_t), .n_fields = 6 },
    {
      { sizeof(void*),  NTPTR,  false, offsetof(heap_t, live_objects) },
      { sizeof(void*),  NTPTR,  false, offsetof(heap_t, grays) },
      { sizeof(size_t), NTUINT, false, offsetof(heap_t, allocated) },
      { sizeof(size_t), NTUINT, false, offsetof(heap_t, alloccap) },
      { sizeof(bool),   NTSINT, false, offsetof(heap_t, read_barrier) },
      { sizeof(bool),   NTSINT, false, offsetof(heap_t, write_barrier) }
    }
  };

dtype_impl_t HeapImpl =
  {
    .value    = Ctype_pointer, .repr    = OBJ,   .data_size=sizeof(void*),
    
    .layout   = &HeapLayout.layout,
  };

type_t HeapType =
  {
    .dtype = &HeapImpl
  };

layout8_t ObjectsLayout =
  {
    ARRAY_LAYOUT( objects, object_t* )
  };

dtype_impl_t ObjectsImpl =
  {
    .value  = Ctype_object,  .repr = OBJ,          .data_size=sizeof(void*),
    .trace  = trace_objects, .free = free_objects, .init     =init_objects,
    .new    = new_stack,

    .layout = &ObjectsLayout.layout
  };

type_t ObjectsType =
  {
    .dtype = &ObjectsImpl
  };

layout8_t BehaviorsLayout =
  {
    ARRAY_LAYOUT( behaviors, behavior_t )
  };

dtype_impl_t BehaviorsImpl =
  {
    .value = Ctype_object, .repr = OBJ, .data_size=sizeof(behavior_t),
    .free  = free_behaviors, .init = init_behaviors,
    .layout = &BehaviorsLayout.layout
  };

type_t BehaviorsType =
  {
    .dtype = &BehaviorsImpl
  };

void rl_obj_memory_init( void )
{
  
  init_dtype( &HeapType, "heap", NOCONS, 0 );
  init_dtype( &ObjectsType, "objects", NOCONS, 0 );
  init_dtype( &BehaviorsType, "behaviors", NOCONS, 0 );

  register_finalize( &Heap, rl_obj_memory_finalize );
}

void rl_obj_memory_finalize( void )
{
  heap_free( &Heap );
  
}
