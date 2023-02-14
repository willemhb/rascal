#ifndef reader_h
#define reader_h

#include "object.h"

/* C types */
typedef enum ReadFl ReadFl;

enum ReadFl {
  READ_READY=0,
  READ_EXPR =1,
  READ_EOS  =2,
  READ_ERR  =3
};

struct Reader {
  Port   *input;
  Bin    *buffer;
  Table  *readers;
  Val    *rp;
  Val     expr;
  ReadFl  flags;
};

/* globals */
extern struct Reader Reader;

/* API */
void  reader_init(void);
void  init_reader(struct Reader *reader);
void  reset_reader(struct Reader *reader, Port *input);

int   rgetc(struct Reader *reader);
int   rpeekc(struct Reader *reader);
int   rputc(struct Reader *reader, int ch);
bool  reof(struct Reader *reader);

int   raccumc(struct Reader *reader);
char *rtoken(struct Reader *reader);

Val   rtakexpr(struct Reader *reader);
Val  *rsavexpr(struct Reader *reader);
void  runsavexprs(struct Reader *reader, usize n_exprs);

void  rsethdlr(struct Reader *reader, Glyph dispatch, Obj *obj);
Obj  *rgethdlr(struct Reader *reader, Glyph dispatch);

#endif
