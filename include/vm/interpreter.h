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
