/**
 *
 * Code for tokenizing Rascal data.
 *
 **/

// headers --------------------------------------------------------------------
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "lang/lex.h"

#include "data/port.h"

#include "sys/error.h"
#include "sys/memory.h"

#include "util/fs.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------
char ScanBuffer[BUFFER_SIZE];

Scanner Lexer = {
  .ios   =NULL,
  .line  =0,
  .offset=0,
  .buffer=ScanBuffer
};

// function prototypes --------------------------------------------------------
bool scanner_is_eof(Scanner* scn);
int  peek_char(Scanner* scn);
int  next_char(Scanner* scn);
void skip_space(Scanner* scn);

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------
bool scanner_is_eof(Scanner* scn) {
  return scn->buffer[scn->offset] == '\0';
}

int peek_char(Scanner* scn) {
  if ( scanner_is_eof(scn) )
    return EOF;

  return scn->buffer[scn->offset];
}

int next_char(Scanner* scn) {
  if ( scanner_is_eof(scn) )
    return EOF;

  int out = scn->buffer[++scn->offset];

  if ( out == '\0' )
    out = EOF;

  return out;
}

void skip_space(Scanner* scn) {
  int c = peek_char(scn);

  while ( c != EOF && (isspace(c) || c == '#') ) {
    if ( c == '\n' )
      scn->line++;

    else if ( c == '#' ) { // skip all the way to end of line or end of file
      while ( c != EOF && c != '\n' )
        c = next_char(scn);

      if ( c == '\n' )
        scn->line++;
    }

    if ( c != EOF )
      c = next_char(scn);
  }
}

// external -------------------------------------------------------------------
void free_scanner(Scanner* scn) {
  free(scn->buffer);
  scn->buffer = 0;
}

void init_scanner(Scanner* scn, Port* ios) {
  // initialize fields
  scn->ios    = ios;
  scn->line   = 1;
  scn->offset = 0;

  // initialize buffer
  scn->buffer = read_stream(ios->ios);
}

Token make_token(Scanner* scn, TokenType type, int length) {
  Token out = {
    .type   = type,
    .value  = scn->buffer + scn->offset,
    .length = length,
    .line   = scn->line
  };

  return out;
}


// initialization -------------------------------------------------------------
