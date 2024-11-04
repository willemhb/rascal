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
  /* type labels */  
  // immediate types
  T_NUL,      // unit type
  T_BOOL,     // true | false
  T_PTR,      // C pointer
  T_GLYPH,    // unicode character
  T_REAL,     // 64-bit float

  // user object types
  T_PORT,     // IO port
  T_FUNC,     // any function
  T_SYM,      // 'symbol
  T_BIN,      // <255 000 123>
  T_PAIR,     // (x . y)
  T_LIST,     // (x y z)
  T_VEC,      // [x y z]
  T_MAP,      // { :x 1, :y 2 }

  // internal object types
  T_BUFFER,   // 
  T_ALIST,
  T_TABLE,
  T_UPVAL,
  T_VNODE,    // vector node
  T_MNODE,    // map node

  // miscellaneous labels
  L_NOTHING,  // marks no label where a label is expected
} Label;

typedef Label Type;

#define NUM_LABLS (L_NOTHING+1)

#endif
