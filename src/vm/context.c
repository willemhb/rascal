#include "util/hashing.h"
#include "util/collection.h"

#include "vm/context.h"

#include "val/table.h"
#include "val/environment.h"

/* Globals */
StringTable StringCache = {
  .table=NULL,
  .cnt  =0,
  .cap  =0
};

GLOBAL_MUT_DICT(ModuleNameCache);
GLOBAL_MUT_DICT(ModulePathCache);
GLOBAL_MUT_DICT(ToplevelModuleDepends);
GLOBAL_MUT_DICT(ToplevelModuleBindings);

Module ToplevelModule = {
  .obj={
    
  },
  
};

RlContext Context = {
  .string_cache     =&StringCache,
  .module_name_cache=&ModuleNameCache,
  .module_path_cache=&ModulePathCache,
  .toplevel_module  =&ToplevelModule,
  .gensym_counter   =0
};

/* Internal APIs */
/* String Table API */


/* External APIs */
