#ifndef rascal_obj_object_h
#define rascal_obj_object_h

#include "obj/type.h"
#include "describe.h"

// the core APIS and types for describing objects go here
// C types
// function pointer types
typedef void      (*trace_t)( object_t *obj );
typedef void      (*free_t)( object_t *obj );
typedef void      (*init_t)( object_t *self, type_t *type, size_t n, void *data );
typedef object_t *(*create_t)( type_t *type, size_t n, void *data );

// structure types
// object
typedef struct obj_impl_t  obj_impl_t;
typedef struct signature_t signature_t;
typedef struct slots_t     slots_t;
typedef struct slot_t      slot_t;
// internal
typedef struct layout_t    layout_t;
typedef struct spec16_t    spec16_t;


struct object_impl_t
{
  OBJECT;

  layout_t *layout;
  object_t *signature;
  object_t *slots;

  trace_t   trace;
  free_t    free;
  init_t    init;
  create_t  create;
};

typedef struct slot_t
{
  OBJECT;

  hash_t      hash;  // TODO: find a way around storing this?
  object_t   *name;  // symbolic name used to reference the slot
  fixnum_t    index; // used to look up field and type information
  object_t   *props; // metadata
} slot_t;

typedef struct slots_t
{
  OBJECT;

  size_t   length;
  size_t   capacity;
  slot_t **data;
} slots_t;

// fixed array of types, representing a signature for a function or type
typedef struct signature_t
{
  OBJECT;

  size_t   length;
  size_t   capacity;
  type_t **data;
} signature_t;

typedef struct spec16_t
{
  uint16_t width:8;
  uint16_t ctype:5;
  uint16_t boxed:3;
  uint16_t offset;
} spec16_t;

struct layout_t
{
  size_t base_size; // total size of all fixed fields 
  size_t n_fields;  // number of slots
  spec16_t specs[];
};

// globals
extern type_t ObjectImplType, SignatureType, SlotsType, SlotType;

// base object methods & utilities go here
// memory api
object_t *create_obj( type_t *type, size_t n, void *data );
void      init_obj( object_t *self, type_t *type, size_t n, void *data );
void      mark_obj( object_t *obj );
void      unmark_obj( object_t *obj );
void      free_obj( object_t *obj );

void      mark_objs( object_t **obj, size_t n );
void      mark_vals( value_t *v, size_t n );

// common allocator types
object_t *create_fixobj( type_t *type, size_t n, void *data ); // ignore n
object_t *create_seqobj( type_t *type, size_t n, void *data ); // n copies of type, or singleton if n is 0

// convenience 
#define rl_trace( x )              GENERIC_2( trace, x )
#define object_type_layout( t )    ( dtype_object_impl( t )->layout )
#define object_type_signature( t ) ( (signature_t*)dtype_object_impl( t )->signature )
#define object_type_slots( t )     ( (slots_t*)dtype_object_impl( t )->slots )
#define object_type_trace( t )     ( dtype_object_impl( t )->trace )
#define object_type_free( t )      ( dtype_object_impl( t )->free )
#define object_type_new( t )       ( dtype_object_impl( t )->new )
#define object_type_init( t )      ( dtype_object_impl( t )->init )

#endif
