#ifndef rl_data_mut_bin_h
#define rl_data_mut_bin_h

/* Mutable 16-bit binary type (used to represent compiled code). */
// headers --------------------------------------------------------------------
#include "common.h"

#include "data/base.h"

// macros ---------------------------------------------------------------------
#define mut_bin_elsize(mb) ((mb)->flags)

// C types --------------------------------------------------------------------
struct MutBin {
  HEAD;

  size_t count, max_count;
  void* data;
};

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
MutBin* mk_mut_bin(size_t o);
void    free_mut_bin(void* ptr);
size_t  mut_bin_write(MutBin* b, size_t o, size_t n, void* data);

// initialization -------------------------------------------------------------
void toplevel_init_data_mut_bin(void);

#endif
