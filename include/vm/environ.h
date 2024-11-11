#ifndef rl_vm_environ_h
#define rl_vm_environ_h

#include "runtime.h"

/* Globals */
// string table (simpler implementation and API)
struct StrTable {
  State* vm;
  size64 cnt, cap, nts;
  Str** strs;
};

/* External API */
// Upv API
void close_upvs(Proc* p, Val* bp);

// StrTable API
void init_str_table(StrTable* st);
void free_str_table(StrTable* st);
void resize_str_table(StrTable* st, size64 n);
void sweep_str_table(StrTable* st);
Str* intern_str(StrTable* st, char* cs, size64 n);

#endif
