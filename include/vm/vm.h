#ifndef rl_vm_vm_h
#define rl_vm_vm_h

/* main headers */
#include "vm/error.h"
#include "vm/memory.h"
#include "vm/value.h"
#include "vm/object.h"

/* main object headers */
#include "vm/obj/heap.h"
#include "vm/obj/reader.h"
#include "vm/obj/vm.h"

/* support object headers */
#include "vm/obj/support/ascii_buffer.h"
#include "vm/obj/support/latin1_buffer.h"
#include "vm/obj/support/utf8_buffer.h"
#include "vm/obj/support/utf16_buffer.h"
#include "vm/obj/support/utf32_buffer.h"

#include "vm/obj/support/objects.h"
#include "vm/obj/support/values.h"
#include "vm/obj/support/stack.h"

#include "vm/obj/support/string.h"
#include "vm/obj/support/bytecode.h"
#include "vm/obj/support/vector.h"

#include "vm/obj/support/readtable.h"

/* commentary

   toplevel initialization and memory management entry points.

   includes everything from this directory/subdirectories to make
   rascal main cleaner. */

void rl_vm_init( void );
void rl_vm_mark( void );

#endif
