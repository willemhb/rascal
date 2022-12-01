#ifndef rl_vm_obj_support_h
#define rl_vm_obj_support_h

/* runtime dispatch module */

#include "vm/obj/support/ascii_buffer.h"
#include "vm/obj/support/bytecode.h"
#include "vm/obj/support/envt.h"
#include "vm/obj/support/gc_object_frame.h"
#include "vm/obj/support/gc_value_frame.h"
#include "vm/obj/support/latin1_buffer.h"
#include "vm/obj/support/namespc.h"
#include "vm/obj/support/objects.h"
#include "vm/obj/support/readtable.h"
#include "vm/obj/support/stack.h"
#include "vm/obj/support/string.h"
#include "vm/obj/support/symbol_table.h"
#include "vm/obj/support/utf16_buffer.h"
#include "vm/obj/support/utf32_buffer.h"
#include "vm/obj/support/utf8_buffer.h"
#include "vm/obj/support/values.h"
#include "vm/obj/support/vector.h"

/* runtime dispatch methods */
void rl_vm_obj_support_init( void );
void rl_vm_obj_support_mark( void );
void rl_vm_obj_support_cleanup( void );

#endif
