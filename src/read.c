#include "runtime.h"
#include "read.h"

// character classes
#define RLSPC  " \n\r\t\v,"
#define DELIM  RLSPC "()[]{}"
#define LOWER  "abcdefghijklmnopqrstuvwxyz"
#define UPPER  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define DIGIT  "0123456789"
#define PUNCT  "?!_-+*/=$^<>"

// globals
Value QuoteSym;

// internal API
// declarations
// helpers
// reader interface
static bool   reof(Vm* vm);
static int    getCh(Vm* vm);
static int    ungetCh(Vm* vm, int ch);
static int    peekCh(Vm* vm);
static void   takeCh(Vm* vm, int ch);
static int    accumCh(Vm* vm, int ch, bool consume);
static ReadFn getReadFn(Vm* vm, int dispatch);
static void   saveExpression(Vm* vm, Value expression);
static void   giveExpression(Vm* vm, Value expression, ReaderState state);
static Value  consumeExpressions(Vm* vm, size_t n, Value (*ctor)(size_t n, Value* args));
static Value  popExpression(Vm* vm);
static Value  takeExpression(Vm* vm);

// errors
static void readError(Vm* vm, const char* fmt, ...);

// predicates and tests
static bool isRlSpace(int ch);
static bool isSymChar(int ch);
static bool tokenMatches(Vm* vm, const char* match);

// toplevel read helper
static Value readExpression(Vm* vm);

// adding dispatch handlers
static void addReaderDispatch(Reader* reader, int dispatch, ReadFn handler);
static void addReaderDispatches(Reader* reader, const char* dispatches, ReadFn handler);

// reader dispatches
void readEOF(Vm* vm, int dispatch);
void readQuote(Vm* vm, int dispatch);
void readSpace(Vm* vm, int dispatch);
void readComment(Vm* vm, int dispatch);
void readAtom(Vm* vm, int dispatch);
void readString(Vm* vm, int dispatch);
void readList(Vm* vm, int dispatch);
void readBits(Vm* vm, int dispatch);
void readNumber(Vm* vm, int dispatch);

// implementations
static bool reof(Vm* vm) {
  return feof(source(vm));
}

static int getCh(Vm* vm) {
  if (reof(vm))
    return EOF;

  return fgetc(source(vm));
}

static int ungetCh(Vm* vm, int ch) {
  if (ch != EOF)
    ungetc(ch, source(vm));

  return ch;
}

static int peekCh(Vm* vm) {
  int next = getCh(vm);

  if (next != EOF)
    ungetCh(vm, next);

  return next;
}

static void takeCh(Vm* vm, int ch) {
  int next = peekCh(vm);
  assert(next == ch);
  getCh(vm);
}

static int accumCh(Vm* vm, int ch, bool consume) {
  assert(ch != EOF);
  assert(ch != '\0');
  writeTextBuffer(readBuffer(vm), ch);

  if (consume) {
    getCh(vm);
    ch = peekCh(vm);
  }

  return ch;
}

static ReadFn getReadFn(Vm* vm, int dispatch) {
  ReadFn out;
  readTableGet(readTable(vm), dispatch, &out);
  return out;
}

static void  giveExpression(Vm* vm, Value expression, ReaderState status) {
  assert(readState(vm) == READER_READY);
  
  if (expression != NOTHING_VAL)
    saveExpression(vm, expression);

  reader(vm)->state = status;
}

static void saveExpression(Vm* vm, Value expression) {
  writeValues(readStack(vm), expression);
}

static Value  consumeExpressions(Vm* vm, size_t n, Value (*ctor)(size_t n, Value* args)) {
  Value* data = readStack(vm)->data;
  Value* args = &data[readStack(vm)->count-n];
  Value  out  = ctor(n, args);

  popValuesN(readStack(vm), n);
  return out;
}

static Value popExpression(Vm* vm) {
  assert(readStack(vm)->count > 0);
  return popValues(readStack(vm));
}

static Value takeExpression(Vm* vm) {
  if (readState(vm) == READER_DONE || readState(vm) == READER_ERROR)
    return NUL_VAL;

  freeTextBuffer(readBuffer(vm));

  Value out         = popExpression(vm);
  reader(vm)->state = READER_READY;

  return out;
}

// errors
static void readError(Vm* vm, const char* fmt, ...) {
  if (panicking(vm))
    return;

  // indicate error
  reader(vm)->state = READER_ERROR;

  // panic
  va_list va;
  va_start(va, fmt);
  vpanic(vm, NOTHING_VAL, NULL, fmt, va);
  va_end(va);
}

// predicates and tests
static bool isRlSpace(int ch) {
  return strchr(RLSPC, ch);
}

static bool isSymChar(int ch) {
  return strchr(UPPER LOWER DIGIT PUNCT, ch);
}

static bool tokenMatches(Vm* vm, const char* match) {
  return strcmp(token(vm), match) == 0;
}

static Value readExpression(Vm* vm) {
  while (!readState(vm)) {
    int dispatch  = peekCh(vm);
    ReadFn readfn = getReadFn(vm, dispatch);

    if (readfn == NULL)
      readError(vm, "Unreadable character %c", dispatch);
    else
      readfn(vm, dispatch);
  }

  return takeExpression(vm);
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
void readEOF(Vm* vm, int dispatch) {
  (void)dispatch;
  giveExpression(vm, NOTHING_VAL, READER_DONE);
}

void readQuote(Vm* vm, int dispatch) {
  (void)dispatch;

  getCh(vm); // clear initial '.

  Value quoted   = read(vm);
  List* expanded = newList2(QuoteSym, quoted);
  giveExpression(vm, TAG_OBJ(expanded), READER_EXPRESSION);
}

void readSpace(Vm* vm, int dispatch) {
  while (isRlSpace(dispatch))
    dispatch = getCh(vm);

  ungetCh(vm, dispatch);
}

void readComment(Vm* vm, int dispatch) {
  while (dispatch != EOF && dispatch != '\n')
    dispatch = getCh(vm);

  if (dispatch != EOF)
    getCh(vm);
}

void readAtom(Vm* vm, int dispatch) {
  while (isSymChar(dispatch))
    dispatch = accumCh(vm, dispatch, true);

  if (tokenMatches(vm, "true"))
    giveExpression(vm, TRUE_VAL, READER_EXPRESSION);

  else if (tokenMatches(vm, "false"))
    giveExpression(vm, FALSE_VAL, READER_EXPRESSION);

  else if (tokenMatches(vm, "nul"))
    giveExpression(vm, NUL_VAL, READER_EXPRESSION);

  else {
    Symbol* sym = getSymbol(token(vm));
    giveExpression(vm, TAG_OBJ(sym), READER_EXPRESSION);
  }
}

void readString(Vm* vm, int dispatch) {
  getCh(vm); // clear opening '"'
  
  while (dispatch != '"') {
    if (dispatch == EOF) {
      readError(vm, "Unexpected EOF reading String");
      break;
    }

    dispatch = accumCh(vm, dispatch, true);
  }

  if (!panicking(vm)) {
    takeCh(vm, '"'); // consume closing '"'
    giveExpression(vm, TAG_OBJ(newString(token(vm), readBuffer(vm)->count)), READER_EXPRESSION);
  }
}

void readList(Vm* vm, int dispatch) {
  size_t i = 0;

  for (dispatch=getCh(vm); dispatch != ')'; i++, dispatch=peekCh(vm) ) {
    if (dispatch == EOF) {
      readError(vm, "Unexpected EOF reading List");
      break;
    }

    Value element = readExpression(vm);
    saveExpression(vm, element);
  }

  if (!panicking(vm)) {
    takeCh(vm, ')'); // consume closing ')'
    giveExpression(vm, consumeExpressions(vm, i, mkListN), READER_EXPRESSION);
  }
}

void readNumber(Vm* vm, int dispatch) {
  while (isdigit(dispatch))
    dispatch = accumCh(vm, dispatch, true);

  if (dispatch == '.') {
    dispatch = accumCh(vm, dispatch, true);

    while (isdigit(dispatch))
      dispatch = accumCh(vm, dispatch, true);
  }

  if (isSymChar(dispatch))
    readAtom(vm, dispatch);

  else
    giveExpression(vm, TAG_NUM(strtod(token(vm), NULL)), READER_EXPRESSION);
}

// external API
void  initReader(Reader* reader) {
  reader->source    = stdin;
  reader->state     = READER_READY;

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
  reader->state = READER_READY;

  freeTextBuffer(&reader->buffer);
  freeValues(&reader->stack);
}

void syncReader(Reader* reader) {
  FILE* ios = reader->source;

  while (!feof(ios))
    fgetc(ios);
}

Value read(Vm* vm) {
  resetReader(&vm->reader);
  return readExpression(vm);
}
