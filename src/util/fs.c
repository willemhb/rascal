/**
 *
 * Miscellaneous IO utilities.
 *
 **/

// headers --------------------------------------------------------------------
#include <errno.h>
#include <string.h>
#include <stdlib.h>

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
   * Read entire file contents into a dynamically allocated string.
   *
   * Stupidly unsafe, but if I wrote it right now there'd be no
   * room to improve.
   *
   **/

  FILE* in = fopen(fname, "rt");

  if ( in == NULL ) {
    fprintf(stderr, "Error reading file '%s': %s.\n", fname, strerror(errno));
    exit(1);
  }

  size_t fsize = file_size(in);
  char*  fdata = malloc((fsize+1) * sizeof(char));

  if ( fdata == NULL ) {
    fprintf(stderr, "Error allocating file buffer: %s\n.", strerror(errno));
    exit(1);
  }

  if ( fgets(fdata, fsize+1, in) == NULL) {
    2
  }
  fclose(in);

  
}

// initialization -------------------------------------------------------------
