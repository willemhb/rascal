/**
 *
 * Miscellaneous IO utilities.
 *
 **/

// headers --------------------------------------------------------------------
#include "util/fs.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------

// external -------------------------------------------------------------------
size_t file_size(FILE* file) {
  /**
   *
   * TODO: more error checking (how many times has this comment appeared in C code).
   *
   **/
  // save original offset
  long opos = ftell(file);

  // advance to end of file
  fseek(file, SEEK_END, SEEK_SET);
  
  long size = ftell(file);

  // reset to original offset
  fseek(file, opos, SEEK_SET);

  // return total size
  return size;
}

char* read_file(char* fname) {
  /**
   *
   * 
   *
   **/
}

// initialization -------------------------------------------------------------
