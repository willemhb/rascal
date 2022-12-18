#ifndef rl_vm_obj_support_sig_h
#define rl_vm_obj_support_sig_h

#include "rascal.h"

/* commentary 

   */

/* C types */
typedef type_t *sig_t;

struct sig_head_t {
  ulong   hash;
  size_t  length;
  type_t *sig[];
};

/* globals */

/* API */
sig_t make_sig( size_t n, type_t *types[n] );
sig_t make_union_sig( size_t n, type_t *types[n] );
sig_t free_sig( size_t n );
size_t sig_length( sig_t sig );
ulong sig_hash( sig_t sig );
int cmp_sigs( sig_t X, sig_t Y );

/* runtime dispatch */
void rl_vm_obj_support_sig_init( void );
void rl_vm_obj_support_sig_mark( void );
void rl_vm_obj_support_sig_cleanup( void );

/* convenience */

#endif
