#ifndef rl_lang_read_h
#define rl_lang_read_h

#include "common.h"

// globals --------------------------------------------------------------------
// forward declarations -------------------------------------------------------
Expr read_exp(RlState* rls, Port* in, int* line);
List* read_file(RlState* rls, char* fname);
List* read_file_s(RlState* rls, char* fname);

#endif
