#include "common.h"
#include "reader.h"
#include "atom.h"

// globals --------------------------------------------------------------------
AtomTable *SymbolTable = NULL;
idno_t     SymbolCount = 0;

Scanner    scanner;
Parser     parser;

// main -----------------------------------------------------------------------
int main( int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  return 0;
}
