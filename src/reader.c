#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

#include "reader.h"

// character classes
#define RLSPC  " \n\r\t\v,"
#define DELIM  RLSPC "()[]{}"
#define LOWER  "abcdefghijklmnopqrstuvwxyz"
#define UPPER  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define DIGIT  "0123456789"
#define PUNCT  "?!_-+*/=$^<>"

// globals
Value QuoteSym;

// generics
#include "tpl/describe.h"

ARRAY_TYPE(TextBuffer, char, int, true);

bool compareReadTableKeys(int cx, int cy) {
  return cx == cy;
}

void internReadTableKey(ReadTable* table, ReadTableEntry* entry, int key, ReadFn* value) {
  (void)table;
  entry->key = key;
  entry->val = *value;
}

uint64_t hashCharacter(int ch) {
  return ch;
}

TABLE_TYPE(ReadTable,
           readTable,
           int,
           ReadFn,
           compareReadTableKeys,
           hashCharacter,
           internReadTableKey,
           '\0',
           NULL);

// internal API
// declarations
// helpers
// reader interface
static bool   reof(Reader* state);
static int    getChar(Reader* state);
static int    ungetChar(Reader* state, int ch);
static int    peekChar(Reader* state);
static int    accumChar(Reader* state, int ch, bool consume);
static ReadFn getReadFn(Reader* state, int dispatch);
static char*  token(Reader* state);
static void   saveExpression(Reader* reader, Value expression);
static void   giveExpression(Reader* reader, Value expression, ReaderState state);
static Value  consumeExpressions(Reader* reader, size_t n, Value (*ctor)(size_t n, Value* args));
static Value  peekExpression(Reader* reader);
static Value  popExpression(Reader* reader);
static Value  takeExpression(Reader* reader);

// errors
static void readError(Reader* state, const char* fmt, ...);

// predicates and tests
static bool isRlSpace(int ch);
static bool isSymChar(int ch);
static bool isAtomicTerminal(int ch);
static bool tokenMatches(Reader* state, const char* match);

// toplevel read helper
static Value readExpression(Reader* state);

// adding dispatch handlers
static void addReaderDispatch(Reader* reader, int dispatch, ReadFn handler);
static void addReaderDispatches(Reader* reader, const char* dispatches, ReadFn handler);

// reader dispatches
void readEOF(Reader* state, int dispatch);
void readQuote(Reader* state, int dispatch);
void readSpace(Reader* state, int dispatch);
void readComment(Reader* state, int dispatch);
void readAtom(Reader* state, int dispatch);
void readString(Reader* state, int dispatch);
void readList(Reader* state, int dispatch);
void readBits(Reader* state, int dispatch);
void readNumber(Reader* state, int dispatch);

// implementations
static bool reof(Reader* state) {
  return feof(state->source);
}

static int getChar(Reader* state) {
  if (reof(state))
    return EOF;

  return fgetc(state->source);
}

static int ungetChar(Reader* state, int ch) {
  if (ch != EOF)
    ungetc(ch, state->source);

  return ch;
}

static int peekChar(Reader* state) {
  int next = getChar(state);

  if (next != EOF)
    ungetChar(state, next);

  return next;
}

static int accumChar(Reader* state, int ch, bool consume) {
  assert(ch != EOF);
  assert(ch != '\0');
  writeTextBuffer(&state->buffer, ch);

  if (consume) {
    getChar(state);
    ch = peekChar(state);
  }

  return ch;
}

static ReadFn getReadFn(Reader* state, int dispatch) {
  ReadFn out;
  readTableGet(&state->table, dispatch, &out);
  return out;
}

static char* token(Reader* state) {
  return state->buffer.data;
}

static void  giveExpression(Reader* reader, Value expression, ReaderState state) {
  assert(reader->state == READER_READY);
  
  if (expression != NOTHING_VAL)
    saveExpression(reader, expression);

  reader->state = state;

  if (state == READER_ERROR) {
    int dispatch = peekChar(reader);

    while (dispatch != EOF)
      dispatch = getChar(reader);
  }
}

static void saveExpression(Reader* reader, Value expression) {
  writeValues(&reader->stack, expression);
}

static Value  consumeExpressions(Reader* reader, size_t n, Value (*ctor)(size_t n, Value* args)) {
  Value* args = &reader->stack.data[reader->stack.count-n];
  Value  out  = ctor(n, args);

  popValuesN(&reader->stack, n);
  return out;
}

static Value peekExpression(Reader* reader) {
  assert(reader->stack.count > 0);

  return reader->stack.data[reader->stack.count-1];
}

static Value popExpression(Reader* reader) {
  assert(reader->stack.count > 0);
  return popValues(&reader->stack);
}

static Value takeExpression(Reader* reader) {
  if (reader->state == READER_DONE || reader->state == READER_ERROR)
    return NUL_VAL;

  freeTextBuffer(&reader->buffer);

  Value out         = popExpression(reader);
  reader->state     = READER_READY;
  reader->panicking = false;

  return out;
}

// errors
static void readError(Reader* reader, const char* fmt, ...) {
  // print the error
  va_list va;
  va_start(va, fmt);
  fprintf(stderr, "read-error: ");
  vfprintf(stderr, fmt, va);
  fprintf(stderr, ".\n");
  va_end(va);

  // set reader state
  giveExpression(reader, NOTHING_VAL, READER_ERROR);
}

// predicates and tests
static bool isRlSpace(int ch) {
  return strchr(RLSPC, ch);
}

static bool isSymChar(int ch) {
  return strchr(UPPER LOWER DIGIT PUNCT, ch);
}

static bool isAtomicTerminal(int ch) {
  return ch == EOF || strchr(RLSPC DELIM ";", ch);
}

static bool tokenMatches(Reader* state, const char* match) {
  return strcmp(token(state), match) == 0;
}

static Value readExpression(Reader* reader) {
  while (!reader->state) {
    int dispatch  = peekChar(reader);
    ReadFn readfn = getReadFn(reader, dispatch);

    if (readfn == NULL)
      readError(reader, "Unreadable character %c", dispatch);
    else
      readfn(reader, dispatch);
  }

  return takeExpression(reader);
}

// adding dispatch handlers
static void addReaderDispatch(Reader* reader, int dispatch, ReadFn handler) {
  readTableSet(&reader->table, dispatch, handler);
}

static void addReaderDispatches(Reader* reader, const char* dispatches, ReadFn handler) {
  for ( ;*dispatches != '\0'; dispatches++ )
    addReaderDispatch(reader, *dispatches, handler);
}

// reader dispatches
void readEOF(Reader* state, int dispatch) {
  (void)dispatch;
  giveExpression(state, NOTHING_VAL, READER_DONE);
}

void readQuote(Reader* state, int dispatch) {
  (void)dispatch;

  getChar(state); // clear initial '.

  Value quoted   = read(state);
  List* expanded = newList2(QuoteSym, quoted);
  giveExpression(state, TAG_OBJ(expanded), READER_EXPRESSION);
}

void readSpace(Reader* state, int dispatch) {
  while (isRlSpace(dispatch))
    dispatch = getChar(state);

  ungetChar(state, dispatch);
}

void readComment(Reader* state, int dispatch) {
  while (dispatch != EOF && dispatch != '\n')
    dispatch = getChar(state);

  if (dispatch != EOF)
    getChar(state);
}

void readAtom(Reader* state, int dispatch) {
  while (isSymChar(dispatch))
    dispatch = accumChar(state, dispatch, true);

  if (tokenMatches(state, "true"))
    giveExpression(state, TRUE_VAL, READER_EXPRESSION);

  else if (tokenMatches(state, "false"))
    giveExpression(state, FALSE_VAL, READER_EXPRESSION);

  else if (tokenMatches(state, "nul"))
    giveExpression(state, NUL_VAL, READER_EXPRESSION);

  else {
    Symbol* sym = getSymbol(token(state));
    giveExpression(state, TAG_OBJ(sym), READER_EXPRESSION);
  }
}

void readString(Reader* state, int dispatch) {
  getChar(state); // clear opening '"'
  
  while (dispatch != '"') {
    if (dispatch == EOF) {
      readError(state, "Unexpected EOF reading String");
      break;
    }

    dispatch = accumChar(state, dispatch, true);
  }

  if (!state->panicking) {
    assert(dispatch == '"');
    getChar(state); // consume closing '"'
    Bits* b = newString(token(state), state->buffer.count);
    giveExpression(state, TAG_OBJ(b), READER_EXPRESSION);
  }
}

void readList(Reader* state, int dispatch) {
  size_t i = 0;

  for (dispatch=getChar(state); dispatch != ')'; i++, dispatch=peekChar(state) ) {
    if (dispatch == EOF) {
      readError(state, "Unexpected EOF reading List");
      break;
    }

    Value element = readExpression(state);
    saveExpression(state, element);
  }

  if (!state->panicking) {
    assert(dispatch == ')');
    getChar(state); // consume closing ')'
    Value list = consumeExpressions(state, i, mkListN);
    giveExpression(state, list, READER_EXPRESSION);
  }
}

void readNumber(Reader* state, int dispatch) {
  while (isdigit(dispatch))
    dispatch = accumChar(state, dispatch, true);

  if (dispatch == '.') {
    dispatch = accumChar(state, dispatch, true);

    while (isdigit(dispatch))
      dispatch = accumChar(state, dispatch, true);
  }

  if (isSymChar(dispatch))
    readAtom(state, dispatch);

  else {
    Number num = strtod(token(state), NULL);
    giveExpression(state, TAG_NUM(num), READER_EXPRESSION);
  }
}

// external API
void  initReader(Reader* reader) {
  reader->source    = stdin;
  reader->state     = READER_READY;
  reader->panicking = false;

  initReadTable(&reader->table);
  initTextBuffer(&reader->buffer);
  initValues(&reader->stack);

  // add dispatches to reader table
  addReaderDispatch(reader, EOF, readEOF);
  addReaderDispatch(reader, '\'', readQuote);
  addReaderDispatch(reader, '(', readList);
  addReaderDispatch(reader, '"', readString);
  addReaderDispatch(reader, ';', readComment);

  addReaderDispatches(reader, " \n\t\r\v,", readSpace);
  addReaderDispatches(reader, DIGIT, readNumber);
  addReaderDispatches(reader, ":" UPPER LOWER PUNCT, readAtom);
}

void freeReader(Reader* reader) {
  freeReadTable(&reader->table);
  freeTextBuffer(&reader->buffer);
  freeValues(&reader->stack);
}

void resetReader(Reader* reader) {
  reader->state     = READER_READY;
  reader->panicking = false;
  
  freeTextBuffer(&reader->buffer);
  freeValues(&reader->stack);
}

Value read(Reader* reader) {
  resetReader(reader);
  return readExpression(reader);
}
