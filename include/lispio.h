#ifndef rascal_lispio_h
#define rascal_lispio_h

#include "common.h"

// C types --------------------------------------------------------------------
typedef enum
  {
    TOK_READY,
    TOK_LPAR,
    TOK_RPAR,
    TOK_LBRACK,
    TOK_RBRACK,
    TOK_HASH,
    TOK_QUOTE,
    TOK_DOT,
    TOK_INTEGER,
    TOK_BOOLEAN,
    TOK_SYMBOL,
    TOK_EOF
  } token_t;

typedef enum
  {
    IOS_INS      = 1,
    IOS_OUTS     = 2,
    IOS_CREATE   = 4,
    
    IOS_TEXT     = 8,
    IOS_BINARY   =16,
    IOS_LISP     =32
  } ios_flags_t;

typedef struct {
  size_t         bufi, bufc;
  schar_t       *buffer;
  ios_t         *stream;
} buffer_t;

typedef struct {
  token_t tok, p_tok;
  value_t val, p_val;

  ios_t *stream;
} scanner_t;

struct ios_t
{
  port_t     id;
  flags_t    flags;
  Ctype_t    ctype;
  uint_t     file_no;

  size_t     refcount; // used to prevent resource leak
  ios_t     *next;     // used by free list of ports

  FILE      *stream;
  buffer_t  *buffer;
  scanner_t *scanner;
  char      *name;
};

struct ios_map_t
{
  size_t used, cap;

  ios_t  *free;    // free list of previously created ports available for reuse
  ios_t **opened;  // all currently opened ports
};

#define REOF (PORT|((value_t)UINT32_MAX))

#define is_ins(ios)   ((ios)->flags&IOS_INS)
#define is_outs(ios)  ((ios)->flags&IOS_OUTS)
#define is_texts(ios) ((ios)->flags&IOS_TEXT)
#define is_bins(ios)  ((ios)->flags&IOS_BIN)

#define STDIN  0
#define STDOUT 1
#define STDERR 2

// utilities ------------------------------------------------------------------
// low level io ---------------------------------------------------------------
port_t  open_port( char *fname, flags_t flags, FILE *f );
sint_t  close_port( port_t p );
ios_t  *get_ios( port_t p, const char *fname );

sint_t  port_getc( port_t ios );
sint_t  port_getb( port_t ios );
sint_t  port_gets( port_t ios, char *buf, size_t buf_size );

sint_t  port_peekc( port_t ios );
sint_t  port_putc( port_t ios, int_t ch );

sint_t  port_puts( port_t ios, char *str, size_t str_size );
sint_t  port_putf( port_t ios, const char *fmt, ... );

// lisp io --------------------------------------------------------------------
value_t lisp_read( port_t p );
value_t lisp_prin( port_t p, value_t x );

// memory management ----------------------------------------------------------
#define INI_BUF_SIZE    512
#define INI_NUM_PORTS   32
#define BUF_GROW_FACTOR 2.0

void trace_ios_map( ios_map_t *iom );

// initernal objects ----------------------------------------------------------
void init_buffer( buffer_t *b, ios_t *stream );
void finalize_buffer( buffer_t *b );
void reset_buffer( buffer_t *b );

void        init_scanner( scanner_t *s, ios_t *owner );
void        reset_scanner( scanner_t *s );

ios_t      *make_ios( ios_map_t *iom );
void        init_ios( ios_t *io, char *name, FILE *stream, flags_t fl );

void        init_ios_map( ios_map_t *iom );
void        finalize_ios_map( ios_map_t *iom );

// initialization -------------------------------------------------------------
void        lispio_init( void );

#endif
