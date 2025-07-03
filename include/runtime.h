#ifndef rl_runtime_h
#define rl_runtime_h

#include "common.h"

// Internal types -------------------------------------------------------------

// function prototypes

// stack & frame operations
void   reset_vals(void);
Expr*  vals_ref(int i);
void   setvp(int n);
Expr*  vpush(Expr x);
Expr*  vrpush(Expr x);
Expr*  vpushn(int n);
Expr   vpop(void);
Expr   vrpop(void);
Expr   vpopn(int n);

void   reset_frames(void);
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
void   reset_vm(void);
UpVal* get_upv(Expr* loc);
void   close_upvs(Expr* base);

// convenience macros


#endif
