#ifndef rl_val_bin_h
#define rl_val_bin_h

#include "val/object.h"

#include "util/text.h"

/* C types */
struct Bin {
  HEADER;

  CType  ctype;
  size32 elsize;
  size64 cnt;
  byte*  bytes;
};

struct MBin {
  HEADER;

  CType  ctype;
  size32 elsize;
  size64 cnt, max;
  byte*  bytes;
};

/* Globals */
extern Type BinType, MBinType;

/* API */
#define is_bin(x)  has_vtype(x, T_BIN)
#define as_bin(x)  ((Bin*)as_obj(x))

Bin*    mk_bin(CType ctype, byte* d, size64 n);
void*   bin_ref(Bin* b, size64 n);
Bin*    bin_set(Bin* b, size64 n, void* d);
Bin*    bin_add(Bin* b, void* d);
size64  bin_cpy(Bin* b, void* d, size64 n);

#define is_mbin(x) has_vtype(x, T_MBIN)
#define as_mbin(x) ((MBin*)as_obj(x))

MBin*   mk_mbin(CType ct);
void    reset_mbin(MBin* b);
void*   mbin_ref(MBin* b, size64 n);
size64  mbin_add(MBin* b, void* d);
size64  mbin_write(MBin* b, size64 n, byte* d);
void    mbin_set(MBin* b, size64 n, void* d);
size64  mbin_cpy(MBin* b, void* d, size64 n);

/* Initialization */
void rl_init_val_bin(void);

#endif
