#include <ctype.h>

#include "reader.h"
#include "val.h"
#include "obj.h"

#include "util/hashing.h"
#include "util/number.h"
#include "util/ios.h"

/* globals */
reader_t Reader;

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

/* API */
/* reader API */
void  give_expression(reader_t *reader, val_t value, read_state_t state);
void  save_expression(reader_t *reader, val_t value);
val_t unsave_expression(reader_t *reader);
val_t take_expression(reader_t *reader);

void  clear_reader(reader_t *reader);
void  init_reader(reader_t *reader);

char *token(reader_t *reader);

void  accumchr(reader_t *reader, char ch);
int   getchr(reader_t *reader);
int   ungetchr(reader_t *reader, int ch);
int   peekchr(reader_t *reader);
int   skipspc(reader_t *reader);

/* reader procedures */
void read_eos(reader_t *reader, int dispatch);
void read_space(reader_t *reader, int dispatch);
void read_number(reader_t *reader, int dispatch);
void read_sym(reader_t *reader, int dispatch);
void read_list(reader_t *reader, int dispatch);

void accumchr(reader_t *reader, char ch) {
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

  if (isrlspc(dispatch))
    read_space(reader, dispatch);

  return peekchr(reader);
}

char* token(reader_t *reader) {
  return reader->buffer->elements;
}

val_t take_expression(reader_t *reader) { 
  val_t out = reader->value;

  clear_reader(reader);

  return out;
}

void  give_expression(reader_t *reader, val_t value, read_state_t state) {
  reader->value = value;
  reader->state = state;
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
      dispatch=fgetc(reader->infile);
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

  while ((dispatch=fgetc(reader->infile)) != EOF && !isrldlm(dispatch) ) {
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
  size_t n = 0;
  
  while (dispatch != EOF && dispatch != '.' && dispatch != ')') {
    val_t x = read_expression(reader);
    save_expression(reader, x);
    n++;
    dispatch = skipspc(reader);
  }

  val_t tail = NUL;

  if (dispatch == '.') {
    tail = read_expression(reader);
    dispatch = skipspc(reader);
  }

  if (dispatch != ')') {
    printf("Unexpected eof reading list.\n");
    abort();
  }

  while (n--) {
    val_t head = unsave_expression(reader);
    tail = cons(head, tail);
  }

  give_expression(reader, tail, read_state_expr);
}

/* initialization */
void add_dispatch(reader_t *reader, int ch, read_fn_t handler) {
  // #ifdef DEBUG
  // printf("table before adding %s: ", show_readtable_char(ch));
  // show_readtable(reader);
  // #endif
  readtable_add(reader->table, ch, handler);
  // #ifdef DEBUG
  // printf("table after adding %s: ", show_readtable_char(ch));
  // show_readtable(reader);
  // #endif
}

void add_dispatches(reader_t *reader, char *chars, read_fn_t handler) {
  for (size_t i=0; i<strlen(chars); i++) {
    // #ifdef DEBUG
    // printf("table before adding %s: ", show_readtable_char(chars[i]));
    // show_readtable(reader);
    // #endif
    readtable_add(reader->table, chars[i], handler);
    // #ifdef DEBUG
    // printf("table after adding %s: ", show_readtable_char(chars[i]));
    // show_readtable(reader);
    // #endif
  }
}

val_t read_expression(reader_t *reader) {
  
  while (reader->state) {
    int dispatch = fgetc(reader->infile);
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

  // debug
  // #ifdef DEBUG
  // show_readtable(&Reader);
  // #endif
}
