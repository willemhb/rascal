#include <stdlib.h>
#include <wctype.h>

#include "read.h"
#include "port.h"
#include "utils.h"
#include "atom.h"
#include "pair.h"

// read table and readtable entries
OBJ_NEW(rentry);

void init_rentry(rentry_t *re, char32_t dispatch, reader_fn_t handler)
{
  re->dispatch = dispatch;
  re->handler  = handler;
}

TABLE_INIT(readt, rentry, READT);
TABLE_FREE(readt, rentry);
TABLE_MARK(readt, rentry);
TABLE_CLEAR(readt, rentry);
TABLE_REHASH(readt, rentry);
TABLE_RESIZE(readt, rentry, READT);
TABLE_PUT(readt, rentry, char32_t, dispatch, hash_long, cmp);
TABLE_GET(readt, rentry, char32_t, dispatch, hash_long, cmp);

void add_reader_macro(readt_t *readt, char32_t dispatch, reader_fn_t handler)
{

  rentry_t *buf;

  if (readt_put(readt, dispatch, &buf))
	init_rentry(buf, dispatch, handler);

  else
    {
      printf( "You absolute dullard, you used the same dispatch character twice!\n" );
      exit(1);
    }
}


static reader_fn_t get_reader(int32_t dispatch)
{
  rentry_t *buf;

  if (!readt_get(&Reader, dispatch, &buf))
    {
	  printf( "No way to read %c, exiting.\n", dispatch );
	  exit( 1 );
    }

  return buf->handler;
}

val_t lisp_read(port_t *port)
{
  while (port_readyp(port))
    {
      int32_t ch  = port_peekc(port);
      reader_fn_t reader = get_reader(ch);
      reader(port, ch);
    }

  return port->value;
}

static bool symchrp(int32_t chr)
{
  if (chr == EOF)
    return false;
  
  return iswalnum(chr) || strchr(":?!+-_*/^", chr);
}

static bool dlmchrp(int32_t chr)
{
  return iswspace(chr) || strchr( "(){}[]", chr );
}

static int testrealchr(int32_t chr, char *acc)
{
  if (iswdigit(chr))
    return 0;
  
  if (dlmchrp(chr))
    return strpbrk(acc, "0123456789") ? 1 : -1;

  if (chr == '.')
    return strchr(acc, '.') ? -1 : 0;

  return -1;
}

static void accumc( port_t *port, char32_t ch )
{
  buffer_push( port->buffer, ch );
}

void read_error(port_t *port, char32_t dispatch)
{
  (void)dispatch;

  val_t errsym = symbol( "error" );
  port_give(port, errsym);
}

void read_atom(port_t *port, char32_t dispatch)
{
  dispatch = port_readc(port); // advance

  // printf( "made it into read_atom.\n" );
  while (symchrp(dispatch))
    {
      accumc( port, dispatch );
      dispatch = port_readc(port);
    }

  // printf( "made it through read.\n" );
  val_t val = symbol(port->buffer->data);
  port_give( port, val );
}

void read_list(port_t *port, char32_t dispatch)
{
  port->temp = NUL;
  port_readc(port);     // advance past opening '('.

  arity_t base = Heap.saved->len;

  // printf( "made it into read list.\n" );

  // arity_t n = 0;

  while ((dispatch=port_peekc(port)) != ')' && dispatch != '.')
    {
      // printf( "made it to the %dth loop in read_list. Dispatch is %c, aka %d.\n", n++, dispatch, dispatch );
      lisp_read(port);
      stack_push( Heap.saved, port_take(port) );
    }

  // printf( "made it this far in read list.\n" );

  if (dispatch == '.')
    {
      port_readc(port); // advance past '.'
      lisp_read(port);
      stack_push(Heap.saved, port_take(port) );
    }

  else
    stack_push(Heap.saved, NUL);

  if (dispatch != ')')
    {
      read_error(port, dispatch);
      return;
    }

  // allocate and initialize
  cons_t *space = new_conses( Heap.saved->len-base-1 );
  init_conses(space, Heap.saved->data+base, Heap.saved->len-base );

  // set token value
  port_give(port, tag_ptr(space, OBJ));

  // cleanup
  resize_stack( Heap.saved, base );
  port_readc(port); // advance past terminal ')'
}

void read_real(port_t *port, char32_t dispatch)
{
  accumc(port, dispatch);

  port_readc(port);            // clear the dispatch character

  int test = 0;

  while (!(test=testrealchr((dispatch=port_peekc(port)), port->buffer->data)))
    accumc( port, dispatch );

  if (test < 0)
      read_atom(port, dispatch);

  else
    {
      char *buf;
      double out = strtod( port->buffer->data, &buf );

      assert(!*buf);

      port_give(port, as_val(out) );
    }
}

void read_quote(port_t *port, char32_t dispatch)
{
  (void)dispatch;
  port_readc(port); // clear '\''

  
}

void read_comment(port_t *port, char32_t dispatch)
{
  while ((dispatch=port_readc(port)) != '\n')
    continue;
}

void read_space(port_t *port, char32_t dispatch)
{
  while (iswspace(dispatch))
    dispatch = port_readc(port);

  // printf( "ungetting '%c'.\n", dispatch );
  port_ungetc(port, dispatch);
}

void read_eof(port_t *port, char32_t dispatch)
{
  (void)dispatch;

  port_give(port, EOS);
}

// toplevel init
void read_init( void )
{
  
  init_readt( &Reader );
  
  add_reader_macro( &Reader, EOF, read_eof );    
  
  for (char dispatch='\0'; dispatch < '\x7f'; dispatch++)
    {
      switch (dispatch)
	{
	case '\0' ... '\b':
	case '\x0e' ... '\x1f':
	case '\x7f':
	  break;

	case '\x09' ... '\x0d':
	case ' ':
	case ',':
	  add_reader_macro( &Reader, dispatch, read_space );
	  break;

	case '+': case '-': case '.': case '0' ... '9':
	  add_reader_macro( &Reader, dispatch, read_real );
	  break;

	case ';':
	  add_reader_macro( &Reader, dispatch, read_comment );
	  break;

	case '(':
	  add_reader_macro( &Reader, dispatch, read_list );
	  break;

	case ')':
	  add_reader_macro( &Reader, dispatch, read_error );
	  break;

	case '\'':
	  add_reader_macro( &Reader, dispatch, read_quote );
	  break;

	default:
	  add_reader_macro( &Reader, dispatch, read_atom );
	  break;
	}
    }
}
