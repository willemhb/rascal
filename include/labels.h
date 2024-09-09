#ifndef rl_labels_h
#define rl_labels_h

#include "common.h"

/* Virtual machine labels Mostly opcodes, but also includes primitive functions, special forms,
   and reader/compiler labels.

   Naming convention:

   E_ = error label
   S_ = syntax or special label (dispatches to compiler label)
   T_ = type label
   O_ = opcode
   R_ = reader label
   C_ = compiler label
   F_ = primitive function label
   L_ = general label
 */

typedef enum Label {
  /* error labels */
  E_OKAY,     // no error
  E_SYS,      // system error
  E_RUN,      // runtime error
  E_READ,     // read error
  E_COMP,     // compile error
  E_STX,      // syntax error
  E_GENFN,    // dispatch error (generic function not found)
  E_EVAL,     // eval error
  E_USER,     // user error
  
  /* special form labels */
  S_CATCH,    // (catch [& a] h b)
  S_DEF,      // (def x y & m)
  S_DO,       // (do x & r)
  S_IF,       // (if t? t) | (if t? t e)
  S_LMB,      // (lmb [& a] b)
  S_PUT,      // (put* n v)
  S_QUOTE,    // (quote x)
  S_THROW,    // (throw e)
  S_CCALL,    // (c-call :c-function :return-type (& :argument-types) (& arguments))
  
  /* type labels */
  // fucked up types,
  T_NONE,     // bottom type
  T_ANY,      // top type
  
  // immediate types
  T_NUL,      // unit type
  T_BOOL,     // true | false
  T_LABL,     // internal label
  T_PTR,      // C pointer
  T_GLYPH,    // unicode character
  T_REAL,     // 64-bit float
  
  // user object types
  T_PORT,     // IO port
  T_FUNC,     // any function
  T_SYM,      // 'symbol
  T_STR,      // "howdy"
  T_BIN,      // <255 000 123>
  T_PAIR,     // (x . y)
  T_LIST,     // (x y z)
  T_VEC,      // [x y z]
  T_MAP,      // { :x 1, :y 2 }
  
  // internal object types
  T_BUFFER,
  T_ALIST,
  T_TABLE,
  T_UPVAL,    // 
  T_VNODE,    // vector node
  T_MNODE,    // map node
  T_STATE,    // global state object
  
  /* opcode labels */
  // miscellaneous
  O_NOOP,  // do nothing
  
  // stack manipulation
  O_POP,  // pop TOS
  O_DUP,  // duplicate TOS
  
  // constant loads
  O_LD_NUL,   // push NUL
  O_LD_TRUE,  // push TRUE
  O_LD_FALSE, // push FALSE
  O_LD_ZERO,  // push ZERO
  O_LD_ONE,   // push ONE
  
  // register loads
  O_LD_FUN, // load currently executing function
  O_LD_ENV, // load current environment
  
  // inlined loads
  O_LD_S16, // load a 16-bit Small that's been inlined in bytecode
  O_LD_S32, // load a 32-bit Small that's been inlined in bytecode
  O_LD_G16, // load a 16-bit Glyph that's been inlined in bytecode
  O_LD_G32, // load a 32-bit Glyph that's been inlined in bytecode
    
  // common loads/stores
  O_LD_VAL,   // load from value store
  O_LD_STK,   // load from stack allocated variable
  O_PUT_STK,  // store into stack allocated variable
  O_LD_UPV,   // load from upvalues
  O_PUT_UPV,  // store to upvalues
  O_LD_GL,
  O_PUT_GL,
  
  // nonlocal control constructs  
  O_CATCH,   // install a frame for a non-resumable effect
  O_UNCATCH, // restore enclosing rp when a `catch` form exits normally without issue

  // branch instructions
  O_JMP,  // unconditional jump
  O_JMPT, // conditional jump (pops TOS)
  O_JMPF, // conditional jump (pops TOS)

  // closure/upvalue instructions
  O_CLOSURE, // create a bound copy of the closure at TOS
  O_CAPTURE, // close upvalues greater than current BP
  
  /* function call instructions (there's lots of these) */
  // call instructions 
  O_CALL0,   // (g ... (f) ...)
  O_CALL1,   // (g ... (f x) ...)
  O_CALL2,   // (g ... (f x y) ...)
  O_CALLN,   // (g ... (f ...) ...)
  
  O_TCALL0,  // (g ... (f x))
  O_TCALL1,  // (g ... (f x))
  O_TCALL2,  // (g ... (f x y))
  O_TCALLN,  // (g ... (f ...))
  
  O_SCALL0,  // (f ... (f))
  O_SCALL1,  // (f ... (f x))
  O_SCALL2,  // (f ... (f x y))
  O_SCALLN,  // (f ... (f ...))
  
  O_APPLY1,  // (g ... (f xs) ...)
  O_APPLY2,  // (g ... (f xs ys) ...)
  O_APPLYN,  // (g ... (f ...) ...)
  
  O_TAPPLY1, // (g ... (f xs))
  O_TAPPLY2, // (g ... (f xs ys))
  O_TAPPLYN, // (g ... (f ...))
  
  O_SAPPLY1, // (f ... (f xs))
  O_SAPPLY2, // (f ... (f xs ys))
  O_SAPPLYN, // (f ... (f ...))
  
  // exit/return/cleanup instructions
  O_RETURN, // restore caller
  O_EXIT,   // exit the virtual machine
  
  /* primfn labels */
  // inlineable primitive functions
  F_TYPEOF, // call `typeof`
  F_HASH,   // call `hash`
  F_SAME,   // call `same?`
  
  // non-inlineable primitive functions (probably need frame space)
  F_READ,   // call `read`
  F_COMP,   // call `comp`
  
  /* read labels */
  R_EOF,    // read EOF
  R_SPACE,  // read whitespace
  R_NUMBER, // read Number
  R_GLYPH,  // read `\<glyph>`
  R_SYM,    // read `sym` (installs new read table?)
  R_STR,    // read Str
  R_BIN,    // read Bin
  R_LIST,   // read List
  R_VEC,    // read Vec
  R_MAP,    // read Map
  R_HASH,   // read `#` (installs new read table)
  R_QUOTE,  // read 'x => (quote x)
  R_TICK,   // read ``` (installs new read table and gensyms)
  R_GSYM,   // read gensym (`#` affixed symbol)
  
  /* compile labels */
  C_EXPR,              // toplevel dispatch
  C_VALUE,
  C_FUNCALL,
  
  /* general labels */  
  L_NEXT,              // get next bytecode instruction
  L_READY,             // indicates initial state
  L_NOTHING           // marks no label where a label is expected
} Label;

typedef Label Error;
typedef Label Type;

#define NUM_ERRS  (E_USER+1)
#define NUM_LABLS (L_NOTHING+1)

#endif
