#ifndef rl_sys_vm_h
#define rl_sys_vm_h

/**
 *
 * Internal VM API.
 * 
 **/

// headers --------------------------------------------------------------------
#include "sys/base.h"

// macros ---------------------------------------------------------------------
#define tos()      (vals_ref(-1)[0])
#define next_op() *(Vm.pc++)

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
Expr*  vals_ref(int i);
void   setvp(int n);
Expr*  vpush(Expr x);
Expr*  vrpush(Expr x);
Expr*  vpushn(int n);
Expr   vpop(void);
Expr   vrpop(void);
Expr   vpopn(int n);

Expr*  frames_ref(int i);
void   setfp(int n);
Expr*  fpush(Expr x);
Expr*  fpushn(int n);
Expr   fpop(void);
Expr   frpop(void);
Expr   fpopn(int n);

// frame manipulation
void   install_fun(Fun* fun, int bp);
void   save_frame(void);
void   restore_frame(void);

// other vm stuff
UpVal* get_upv(Expr* loc);
void   close_upvs(Expr* base);

// initialization -------------------------------------------------------------

#endif
