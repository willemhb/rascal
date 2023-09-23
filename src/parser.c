#include <stdlib.h>
#include <string.h>

#include "object.h"
#include "parser.h"

// internal API


// external API
void initParser(Parser* parser) {
  parser->offset     = 0;
  parser->hadError   = false;
  parser->expression = NOTHING_VAL;
}
