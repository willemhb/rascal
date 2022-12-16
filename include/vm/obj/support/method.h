#ifndef rl_vm_obj_support_method_h
#define rl_vm_obj_support_method_h

#include "rascal.h"

#include "vm/obj/support/signature.h"

/* commentary */

/* C types */
typedef struct method_t method_t;

typedef enum method_type_t method_type_t;

enum method_type_t
  {
    method_type_primitive,
    method_type_native,
    method_type_user
  };

struct method_t
{
  // "slow" table implemented as invasive linked list (obviously very slow)
  method_t     *next;

  // signature
  type_t       *return_type;
  signature_t   argument_types;

  method_type_t method_type;
  bool          accepts_vargs;

  union
  {
    ushort      as_primitive;
    native_t    as_native;
    rl_code_t  *as_code;
  };
};

/* globals */

/* API */
method_t *assoc_method(method_t *methods, signature_t signature);

/* runtime dispatch */
void rl_vm_obj_support_method_init( void );
void rl_vm_obj_support_method_mark( void );
void rl_vm_obj_support_method_cleanup( void );

/* convenience */

#endif
