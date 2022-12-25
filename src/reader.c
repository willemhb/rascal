#include <ctype.h>

#include "reader.h"
#include "prin.h"

#include "val.h"
#include "obj.h"

#include "list.h"
#include "sym.h"
#include "vec.h"
#include "code.h"

#include "util/hashing.h"
#include "util/number.h"
#include "util/ios.h"

/* globals */
reader_t Reader;

val_t CodeSym, VecSym;

/* instantiations */
#include "tpl/impl/buffer.h"
#include "tpl/impl/hashmap.h"

BUFFER(readbuffer, ascii_t, int, pad_stack_size);

bool readt_cmp(int x, int y) { return x == y; }
read_fn_t readt_intern(int dispatch, void **space)
{
  (void)dispatch;
  (void)space;
  return NULL;
}

void readt_noval(reader_t *reader, int dispatch) {
  (void)reader;

  printf("Unexpected token '%c' reading file.\n", dispatch);
  abort();
}

HASHMAP(readtable, int, read_fn_t, pad_table_size, hash_int32, readt_cmp, readt_intern, -2, readt_noval);

/* debugging */
#define show_dispatch(d) printf("%s:%d:%s: dispatch='%c'\n", __FILE__,__LINE__,__func__,(d))

/* API */
/* reader API */
void  give_expression(reader_t *reader, val_t value, read_state_t state);
void  give_expressions(reader_t *reader, val_t value, read_state_t state, size_t n);
void  save_expression(reader_t *reader, val_t value);
val_t take_expression(reader_t *reader);

void  clear_reader(reader_t *reader);
void  init_reader(reader_t *reader);

char *token(reader_t *reader);

void  accumchr(reader_t *reader, int ch);
int   getchr(reader_t *reader);
int   ungetchr(reader_t *reader, int ch);
int   peekchr(reader_t *reader);
int   skipspc(reader_t *reader);

/* reader procedures */
int  read_seq(reader_t *reader, char *type, char *term);
void read_eos(reader_t *reader, int dispatch);
void read_space(reader_t *reader, int dispatch);
void read_number(reader_t *reader, int dispatch);
void read_sym(reader_t *reader, int dispatch);
void read_list(reader_t *reader, int dispatch);
void read_vec(reader_t *reader, int dispatch);
void read_obj(reader_t *reader, int dispatch);

/* reader helpers */
int read_seq(reader_t *reader, char* type, char *term) {
  int dispatch;
  
  while ((dispatch=skipspc(reader)) != EOF && !strchr(term, dispatch)) {
    val_t x = read_expression(reader);
    save_expression(reader, x);
  }

  if (dispatch == EOF) {
    printf("Unexpected eos reading %s.\n", type);
    abort();
  } else {
    getchr(reader); // take terminal token
  }

  return dispatch;
}

void accumchr(reader_t *reader, int ch) {
  if (ch != EOF)
    readbuffer_push(reader->buffer, ch);
}

int getchr(reader_t *reader) {
  return getc(reader->infile);
}

int ungetchr(reader_t *reader, int ch) {
  if (ch == EOF)
      give_expression(reader, NUL, read_state_eos);

  else
    return ungetc(ch, reader->infile);

  return ch;
}

int peekchr(reader_t *reader) {
  int ch = getchr(reader);
  
  ungetchr(reader, ch);

  return ch;
}

int skipspc(reader_t *reader) {
  int dispatch = peekchr(reader);

  if (isrlspc(dispatch)) {
    read_space(reader, dispatch);
  }

  dispatch = peekchr(reader);
  return dispatch;
}

char* token(reader_t *reader) {
  return reader->buffer->elements;
}

val_t take_expression(reader_t *reader) { 
  val_t out = reader->value;

  clear_reader(reader);

  return out;
}

void give_expression(reader_t *reader, val_t value, read_state_t state) {
  reader->value = value;
  reader->state = state;
}

void give_expressions(reader_t *reader, val_t value, read_state_t state, size_t n) {
  reader->value = value;
  reader->state = state;
  vals_popn(reader->subx, n);
}

void save_expression(reader_t *reader, val_t val) {
  vals_push(reader->subx, val);
}

val_t unsave_expression(reader_t *reader) {
  return vals_pop(reader->subx);
}

void clear_reader(reader_t *reader) {
  give_expression(reader, NUL, read_state_ready);
  reset_readbuffer(reader->buffer);
}

void  init_reader(reader_t *reader) {
  reader->infile = stdin;
  reader->state  = read_state_ready;
  reader->buffer = make_readbuffer(0);
  reader->table  = make_readtable(0, NULL);
  reader->subx   = make_vals(0, NULL);
}

/* handlers */
void read_eos(reader_t *reader, int dispatch) {
  (void)dispatch;

  give_expression(reader, NUL, read_state_eos);
}

void read_space(reader_t *reader, int dispatch) {
  while ( isspace(dispatch) ) {
    dispatch=getchr(reader);
    }

  if (dispatch == EOF)
    read_eos(reader, dispatch);

  else
    ungetchr(reader, dispatch);
}

void read_symbol(reader_t *reader, int dispatch) {
  accumchr(reader, dispatch);

  while (isrlsymchr(dispatch=getchr(reader))) {
    accumchr(reader, dispatch);
  }

  ungetchr(reader, dispatch);

  val_t symbol = sym(token(reader));
  
  give_expression(reader, symbol, read_state_expr);
}

void read_number(reader_t *reader, int dispatch) {
  accumchr(reader, dispatch);

  bool valid = true;

  while ((dispatch=getchr(reader)) != EOF && !isrldlm(dispatch) ) {
    // show_dispatch(dispatch);
    switch (dispatch) {
    case '0' ... '9':
      break;

    case '.':
      valid = valid && !strchr(token(reader), '.');
      break;

    default:
      valid = false;
      break;
    }

    accumchr(reader, dispatch);
  }

  ungetchr(reader, dispatch);

  val_t result;

  if (valid) {
    char *buf;
    
    real_t x = strtod(token(reader), &buf);

    if (*buf == '\0')
      result = as_val(x);

    else
      result = sym(token(reader));

  } else {
    result = sym(token(reader));
  }

  give_expression(reader, result, read_state_expr);
}

void read_list(reader_t *reader, int dispatch) {
  size_t base = reader->subx->len;
  
  dispatch = read_seq(reader, "list", ".)");

  if (dispatch == '.') {
    val_t tail = read_expression(reader);
    save_expression(reader, tail);
    dispatch = skipspc(reader);
  } else {
    save_expression(reader, NUL);
  }

  if (dispatch != ')') {
    printf("Unexpected eof reading list.\n");
    abort();
  } else {
    getchr(reader); // take ')'
  }

  cons_t c = make_list(reader->subx->len, (val_t*)reader->subx->elements);
  val_t out = tag_val(c, OBJECT);

  give_expressions(reader, out, read_state_expr, reader->subx->len-base);
}

void read_vec(reader_t *reader, int dispatch) {
  (void)dispatch;
  size_t base = reader->subx->len;

  read_seq(reader, "vec", "]");

  vec_t v = make_vec(reader->subx->len, (val_t*)reader->subx->elements);
  val_t out = tag_val(v, OBJECT);

  give_expressions(reader, out, read_state_expr, reader->subx->len-base);
}

void read_obj(reader_t *reader, int dispatch) {
  (void)dispatch;
  size_t base = reader->subx->len;

  val_t head = read_expression(reader), out;

  if (!is_sym(head)) {
    printf("Unexpected expression in object literal: ");
    prin(head);
    printf(".\n");
    abort();
  }

  if ((dispatch=getchr(reader)) != '(') {
    printf("Unexpected character in %s literal: '%c'.\n", as_sym(head), dispatch);
    abort();
  }

  read_seq(reader, as_sym(head), ")");

  if (head == CodeSym) {
    code_t c = code_from_obj(reader->subx->len, (val_t*)reader->subx->elements);
    out = tag_val(c, OBJECT);

  } else if (head == VecSym) {
    vec_t v = make_vec(reader->subx->len, (val_t*)reader->subx->elements);
    out = tag_val(v, OBJECT);

  } else {
    printf("Cant' read unknown object type '%s'.\n", as_sym(head));
    abort();
  }

  give_expressions(reader, out, read_state_expr, base-reader->subx->len);
}

/* initialization */
void add_dispatch(reader_t *reader, int ch, read_fn_t handler) {
  readtable_add(reader->table, ch, handler);
}

void add_dispatches(reader_t *reader, char *chars, read_fn_t handler) {
  for (size_t i=0; i<strlen(chars); i++) {
    readtable_add(reader->table, chars[i], handler);
  }
}

val_t read_expression(reader_t *reader) {
  
  while (reader->state) {
    int dispatch = getchr(reader);
    read_fn_t handler = readtable_get(reader->table, dispatch);

    assert(handler);
    handler(reader, dispatch);
  }

  return take_expression(reader);
}

/* initialization */
void reader_init(void)
{
  init_reader(&Reader);

  // add reader procedures
  add_dispatches(&Reader, SYMBOLS, read_symbol);
  add_dispatches(&Reader, DIGITS, read_number);
  add_dispatches(&Reader, RLSPACES, read_space);
  add_dispatch(&Reader, EOF, read_eos);
  add_dispatch(&Reader, '(', read_list);
  add_dispatch(&Reader, '[', read_vec);
  add_dispatch(&Reader, '#', read_obj);

  // add object names
  CodeSym = sym("code");
  VecSym  = sym("vec");
}
