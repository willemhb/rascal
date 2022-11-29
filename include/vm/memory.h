#ifndef rl_vm_memory_h
#define rl_vm_memory_h

#include "rascal.h"

#include "vm/obj/heap.h"

/* commentary */

/* C types */

/* globals */

/* API */
void *alloc( size_t n_bytes );
void *alloc_array( size_t count, size_t ob_size );
void *alloc_table( size_t base, size_t count, size_t ob_size );

void  dealloc( void *ptr, size_t n_bytes );
void  dealloc_array( void *ptr, size_t count, size_t ob_size );
void  dealloc_table( void *ptr, size_t base, size_t count, size_t ob_size );

void *adjust( void *ptr, size_t old_size, size_t new_size );
void *adjust_array( void *ptr, size_t old_count, size_t new_count, size_t ob_size );
void *adjust_table( void *ptr, size_t base, size_t old_count, size_t new_count, size_t ob_size );

void  collect_garbage( void );

/* runtime */
void rl_vm_memory_init( void );
void rl_vm_memory_mark( void );

/* convenience */

#endif
