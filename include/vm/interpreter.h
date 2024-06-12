/* Dumping ground for code that needs reorganizing */

typedef struct RFrame RFrame;
typedef struct CFrame CFrame;
typedef struct IFrame IFrame;

typedef struct RState RState;
typedef struct CState CState;
typedef struct IState IState;


struct RState {
  Alist        frames;
  MutVec       stack;
  MutBin       buffer;
  ReadTable*   rt;
  MutMap*      gs;
  Port*        input;
};

struct CState {
  Alist        frames;
  MutVec       stack;
  List*        form;
  Environ*     envt;
  MutVec*      constants;
  MutBin*      code;
};

struct IState {
  Alist        frames;
  MutVec       stack;
  UpValue*     upvals; // List of open upvalues
  Closure*     code;   // currently executing code object
  uint16_t*    pc;     // program counter for currently executing code object
  size_t       cp;     // catch pointer
  size_t       hp;     // handle pointer
  size_t       bp;     // base pointer
  size_t       fp;     // frame pointer
};


/* RState and reader APIs */
rl_status_t rstate_mark(RState* s);
rl_status_t rstate_push(RState* s, Value v);
rl_status_t rstate_write(RState* s, Value* vs, size_t n);
rl_status_t rstate_pushn(RState* s, size_t n, ...);
rl_status_t rstate_pushf(RState* s, Port* i, ReadTable* rt, MutMap* gs);
rl_status_t rstate_writef(RState* s, RFrame* f, size_t n);
rl_status_t rstate_popf(RState* s);
rl_status_t rstate_writec(RState* s, char c);
rl_status_t rstate_writecs(RState* s, char* cs, size_t n);
rl_status_t rstate_pop(RState* s, Value* b);
rl_status_t rstate_popn(RState* s, Value* b, bool t, size_t n);

/* CState and compiler APIs */
rl_status_t cstate_mark(CState* s);
rl_status_t cstate_pop(CState* s, Value* b);

/* IState and interpreter APIs */
rl_status_t istate_mark(IState* s);
rl_status_t istate_pop(IState* s, Value* b);
