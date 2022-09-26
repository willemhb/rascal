#ifndef rascal_envt_h
#define rascal_envt_h

#include "obj.h"
#include "obj/cons.h"
#include "obj/table.h"

typedef struct envt_t envt_t;
typedef struct func_t func_t;

typedef enum
  {
   var_fl_closure =1,
   var_fl_module  =2,
   var_fl_toplevel=4,
   var_fl_captured=8,
  } var_fl_t;

typedef enum
  {
    envt_fl_closure =1,
    envt_fl_moduel  =2,
    envt_fl_toplevel=4,
  } envt_fl_t;

struct envt_t
{
  OBJ_HEAD;

  envt_t  *next;

  func_t  *module;
  table_t *local;
  table_t *global;
  table_t *macro;
};

// globals --------------------------------------------------------------------
extern envt_t *ToplevelNS;

// forward declarations

#endif
