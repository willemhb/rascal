#include "read.h"
#include "runtime.h"

#include "util/io.h"
#include "util/string.h"

// globals ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define ALPHA    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define NUMERIC  "0123456789"
#define ALNUM    ALPHA NUMERIC
#define SPACE    " \n\v\f\r"
#define INTRASYM ALPHA "+-*/<=>_|:!?&$"
#define SYMCHR   INTRASYM NUMERIC

struct Reader Reader = {
  .dispatch   ={
    .obj ={
      .next =NULL,
      .type =TABLE,
      .flags=NOFREE|GRAY|IDTABLE,
    },
    .cnt =0,
    .cap =0,
    .data=NULL,
    .ord =NULL
  },
  .expressions={
    .obj ={
      .next =NULL,
      .type =VECTOR,
      .flags=NOFREE|GRAY,
    },
    .cnt =0,
    .cap =0,
    .data=NULL
  },
  .buffer     ={
    .obj ={
      .next =NULL,
      .type =BINARY,
      .flags=NOFREE|GRAY,
    },
    .cnt    =0,
    .cap    =0,
    .data   =NULL,
    .elSize =1,
    .encoded=true
  },
  .expression =NIL,
  .infile     =NULL,
  .state      =READY_TOKEN
};

// internal API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void    accumulate_expression( void );
value_t take_expression( void );
void    read_sequence( FILE* ios, int term );
void    read_list( FILE* ios, int disp );
void    read_symbol( FILE* ios, int disp );
void    read_number( FILE* ios, int disp );
void    read_quote( FILE* ios, int disp );
void    read_space( FILE* ios, int disp );
void    read_comment( FILE* ios, int disp );
void    read_eof( FILE* ios, int disp );

void accumulate_expression( void ) {
  value_t expr = take_expression();
  vector_push(&Reader.expressions, Reader.expression);
}

value_t take_expression( void ) {
  value_t out = Reader.expression;
  Reader.expression = NIL;
  Reader.state = READY_TOKEN;
  reset_binary(&Reader.buffer);
  return out;
}

void give_expression( value_t expr, token_t token ) {
    Reader.state      = token;
    Reader.expression = expr;
}

void add_reader_dispatch( int ch, void (*fptr)( FILE* ios, int disp ) ) {
  table_set( &Reader.dispatch, glyph(ch), pointer(fptr) );
}

void add_reader_dispatches( char* chs, void (*fptr)( FILE* ios, int disp )) {
  while ( *chs ) {
    add_reader_dispatch(*chs, fptr);
    chs++;
  }
}

bool is_symbol_character( int ch ) {
  return strchr(SYMCHR, ch);
}

void read_sequence( FILE* ios, int term ) {
  
}

// external API +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void reset_reader( struct Reader* reader ) {
  if ( reader->dispatch.cap == 0 )
    reset_table(&reader->dispatch);

  reset_vector(&reader->expressions);
  reset_binary(&reader->buffer);
  reader->expression = NIL;
  reader->infile = NULL;
  reader->state = INIT_TOKEN;
}

void init_reader( struct Reader* reader, FILE* infile ) {
  if ( reader->state != INIT_TOKEN )
    reset_reader(reader);

  reader->infile = infile;
  reader->state  = READY_TOKEN;
}

value_t read( FILE* ios ) {
  while ( Reader.state ) {
    int disp = fpeekc(ios);
    value_t dispGl = glyph(disp);
    value_t reader = table_get(&Reader.dispatch, dispGl);
    forbid("read", reader == NOTFOUND, dispGl, "unreadable glyph");
    void (*fptr)( FILE* ios, int disp ) = as_pointer(reader);
    fptr(ios, disp);
  }

  return Reader.expression;
}

void reader_init( void ) {
  init_reader(&Reader, stdin);

  add_reader_dispatch('(', read_list);
  add_reader_dispatches(INTRASYM, read_symbol);
  add_reader_dispatch('\'', read_quote);
  add_reader_dispatches(SPACE"\t,", read_space);
  add_reader_dispatch(';', read_comment);
  add_reader_dispatch(EOF, read_eof);
}
