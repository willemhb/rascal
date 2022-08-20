#include <errno.h>
#include <string.h>

#include "memutils.h"

#include "lispio.h"
#include "runtime.h"
#include "memory.h"

// global backing objects -----------------------------------------------------
ios_map_t PortsObject;

// forward declarations -------------------------------------------------------
void        grow_buffer( buffer_t *b );
void        shrink_buffer( buffer_t *b );
bool_t      check_buffer_grow( buffer_t *b, size_t n_chars );
bool_t      check_buffer_shrink( buffer_t *b, size_t n_chars );
size_t      buffer_putc( buffer_t *b, int_t ch );
size_t      buffer_write( buffer_t *b, char_t *s, size_t n_chars );
void        grow_ios_map( ios_map_t *iom );
void        free_ios( ios_t *io );

// io api ---------------------------------------------------------------------
sint_t close_port( port_t p )
{
  ios_t *ios = Ports->opened[p];

  // this should
  require( ios,
	   tagint(p, PORT),
	   "close",
	   "port %d already closed" );

  Ports->opened[p] = NULL;
  ios->next        = Ports->free;
  Ports->free      = ios;
  fclose( ios->stream );
  return 0;
}

port_t open_port(char *fname, flags_t flags, FILE *f)
{
  ios_t *new;
  
  if (f == NULL)
    {
      char *mode;

      if (flags&IOS_INS)
	{
	  if (flags&IOS_TEXT)
	    mode = "rt";

	  else
	    mode = "rb";
	}

      else
	{
	  if (flags&IOS_TEXT)
	    if (flags&IOS_CREATE)
	      mode = "wt+";
	    else
	      mode = "wt";
	  else
	    if (flags&IOS_CREATE)
	      mode = "wb+";
	    else
	      mode = "wb";
	}

      f = fopen( fname, mode );
      
      require( f,
	       REOF,
	       "open",
	       "error opening %s: %s.",
	       fname,
	       strerror(errno) );
    }

  new = make_ios( Ports );
  init_ios( new, fname, f, flags );

  return new->id;
}

ios_t *get_ios( port_t p, const char *fname )
{
  ios_t *out = Ports->opened[p];

  require( out,
	   tagint( p, PORT ),
	   fname,
	   "port %d has already closed",
	   p );
  
  return out;
}

sint_t port_getc( port_t p )
{
  ios_t *ios    = get_ios( p, "getc" );

  require( is_ins(ios),
	   tagint( p, PORT ),
	   "getc",
	   "port %d is not an input port",
	   p );

  require( is_texts(ios),
	   tagint( p, PORT ),
	   "getc",
	   "port %d is not a text port",
	   p );
  
  FILE  *stream = ios->stream;
  
  if (feof(stream))
    return EOF;

  return fgetc( stream );
}

sint_t port_gets( port_t p, char *buf, size_t buf_size )
{
  ios_t *ios = get_ios( p, "gets" );

  require( is_ins(ios),
	   tagint( p, PORT ),
	   "gets",
	   "port %d is not an input port" );

  require( is_texts(ios),
	   tagint( p, PORT ),
	   "gets",
	   "port %d is not a text port" );

  
}

// implementations ------------------------------------------------------------
void init_buffer( buffer_t *b, ios_t *owner )
{
  b->bufi   = 0;
  b->bufc   = INI_BUF_SIZE;
  b->buffer = malloc_s( INI_BUF_SIZE );
  b->stream = owner;
}

void init_scanner( scanner_t *s, ios_t *owner )
{
  s->stream = owner;
  reset_scanner( s );
}

void reset_scanner( scanner_t *s )
{
  s->val = s->p_val = NIL;
  s->tok = s->p_tok = TOK_READY;
}

void init_ios( ios_t *ios, char *name, FILE *stream, flags_t fl )
{
  ios->flags    = fl;
  ios->ctype    = (fl&IOS_BINARY) ? C_uint8 : C_ascii;
  ios->file_no  = fileno( stream );
  ios->refcount = 0;
  ios->next     = NULL;
  ios->stream   = stream;

  if (ios->buffer == NULL)
    ios->buffer   = (buffer_t*)((char*)ios + sizeof(ios_t));

  if (ios->scanner == NULL)
    ios->scanner  = (scanner_t*)((char*)ios + sizeof(ios_t) + sizeof(scanner_t));
  
  ios->name     = name;

  init_buffer(  ios->buffer, ios );
  init_scanner( ios->scanner, ios );
}

ios_t *make_ios( ios_map_t *iom )
{
  ios_t *out;
  
  if (iom->free)
    {
      out       = iom->free;
      iom->free = out->next;
      out->next = NULL;      // avoid any chaos caused by retaining this reference
    }

  else
    {
      out = malloc_s( sizeof(ios_t)    +
		      sizeof(buffer_t) +
		      sizeof(scanner_t) );

      out->id = iom->used++;

      if (iom->used >= iom->cap)
	grow_ios_map( iom );
    }

  iom->opened[out->id] = out;

  return out;
}

void grow_ios_map( ios_map_t *m )
{
  m->cap    *= 2;
  m->opened  = realloc_s( m->opened, m->cap * sizeof(ios_t*) );
}

void init_ios_map( ios_map_t *m )
{
  m->used   = 0;
  m->cap    = INI_NUM_PORTS;
  m->free   = NULL;
  m->opened = malloc_s( m->cap*sizeof(ios_t*) );
}

void finalize_ios_map( ios_map_t *m )
{
  // free opened ports
  for (size_t i=0; i<m->used; i++)
    {
      if (m->opened[i])
	{
	  free_ios(m->opened[i]);
	  m->opened[i] = NULL;
	}
    }

  // free unused ports in the free list
  while (m->free)
    {
      ios_t *ios = m->free;
      m->free    = ios->next;
      
      free_ios( ios );
    }

  // free the opened array
  free_s( m->opened );
  m->opened = NULL;
}

void trace_ios_map( ios_map_t *m )
{
  /* to avoid leaking file handles, we check during garbage collection

     whether there are any live references to a port remaining, using

     a simple reference counting strategy. If a port's reference count is 0,
     the ios object is cleaned up and it's made available for reuse.

     The ios map is also traced in this pass. 
  */
  
  // don't accidentally try to close the standard streams
  for (size_t i=3; i<m->used; i++)
    {
      if (m->opened[i] == NULL)
	continue;

      if (m->opened[i]->refcount)
	{
	  m->opened[i]->refcount = 0; // reset for next cycle

	  if (m->opened[i]->flags & IOS_LISP)
	    {
	      scanner_t *s = m->opened[i]->scanner;

	      value_t tmp = s->val;
	      tmp         = relocate( tmp );
	      s->val      = tmp;
	      tmp         = s->p_val;
	      tmp         = relocate( tmp );
	      s->p_val    = tmp;
	    }
	}

      else
	{
	  ios_t *unused = m->opened[i];

	  fclose( unused->stream );    // clean up file handle
	  
	  unused->next  = m->free;     // link existing free list
	  m->free       = unused;      // add to free list
	  m->opened[i]  = NULL;        // invalidate reference
	}
    }
}

// initialization -------------------------------------------------------------
void lispio_init( void )
{
  Ports = &PortsObject;

  init_ios_map( Ports );

  /* create stream objects */
  open_port( "<ins>",  IOS_INS  | IOS_TEXT, stdin );
  open_port( "<outs>", IOS_OUTS | IOS_TEXT, stdout );
  open_port( "<errs>", IOS_OUTS | IOS_TEXT, stderr );

  /* bind */
}
